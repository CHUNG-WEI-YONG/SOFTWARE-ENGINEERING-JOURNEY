from dataclasses import dataclass
from datetime import datetime
from typing import Optional ,Dict ,Any
import os 

class ContextPacket:
    content:str
    time_stamp:datetime
    token_count:int=0
    relevant_score:float=0.5
    metadata:Optional[Dict[str,Any]]=None

    def _post_init_(self):
        if self.metadata is None:
            self.metadata={}
        self.relevant_score=max(0.0,min(1.0,self.relevant_score))


@dataclass
class ContextConfig:
    max_tokens:int =5000
    min_relevance:float=0.1
    reserve_ratio:float=0.2
    enable_compression:bool=True
    recency_weight:float=0.3
    relevance_weight:float=0.7

    def __post_init__(self):
        """验证配置参数"""
        assert 0.0 <= self.reserve_ratio <= 1.0, "reserve_ratio 必须在 [0, 1] 范围内"
        assert 0.0 <= self.min_relevance <= 1.0, "min_relevance 必须在 [0, 1] 范围内"
        assert abs(self.recency_weight + self.relevance_weight - 1.0) < 1e-6, \
            "recency_weight + relevance_weight 必须等于 1.0"



class ContextBuilder:
    def _gather_(self,
    user_query: str,
    conversation_history: Optional[list[message]] = None,
    system_instructions: Optional[str] = None,
    custom_packets: Optional[list[ContextPacket]] = None) -> list[ContextPacket]:

        packets=[]
        if system_instructions:
            packets.append(ContextPacket( content=system_instructions,
            timestamp=datetime.now(),
            token_count=self._count_tokens(system_instructions),
            relevance_score=1.0,  # 系统指令始终保留
            metadata={"type": "system_instruction", "priority": "high"}))

        if self.memory_tool:
            try:
                memory_results=self.memory_tool.run({
                    "action":"search",
                    "query":"user_query",
                    "limit":10,
                    "min_importance":0.3
                })
                memory_packets = self._parse_memory_results(memory_results, user_query)
                packets.extend(memory_packets)
            except Exception as e:
                print("RAG error in finding memory")

        if self.rag_tool:
            try:
                rag_results = self.rag_tool.run({
                "action": "search",
                "query": user_query,
                "limit": 5,
                "min_score": 0.3
                })
            # 解析 RAG 结果并转换为 ContextPacket
                rag_packets = self._parse_rag_results(rag_results, user_query)
                packets.extend(rag_packets)
            except Exception as e:
                print(f"[WARNING] RAG 检索失败: {e}")

        if conversation_history:
            recent_history = conversation_history[-5:]  # 默认保留最近 5 条
            for msg in recent_history:
                packets.append(ContextPacket(
                content=f"{msg.role}: {msg.content}",
                timestamp=msg.timestamp if hasattr(msg, 'timestamp') else datetime.now(),
                token_count=self._count_tokens(msg.content),
                relevance_score=0.6,  # 历史消息的基础相关性
                metadata={"type": "conversation_history", "role": msg.role}
            ))

        if custom_packets:
            packets.extend(custom_packets)

        print(f"[ContextBuilder] 汇集了 {len(packets)} 个候选信息包")
        return packets

    def _select__(self,packets:list[ContextPacket],user_query:str,available_tokens:int):
        system_packets=[p for p in packets if p.metadata.get("type")=="system_instruction"]
        other_packets=[p for p in packets if p.metadata.get("type")!="system_instruction"]

        system_tokens=sum(p.token_count for p in system_packets)
        remaining_tokens = available_tokens - system_tokens

        if remaining_tokens <= 0:
            print("[WARNING] 系统指令已占满所有 token 预算")
            return system_packets

        score_packets=[]
        for p in other_packets:
            if p.relevant_score==0.5:
                relavance=self._calculate_relevance_score(p.content,user_query)
                p.relevant_score=relavance

            recency = self._calculate_recency(p.timestamp)
            combined_score = (
            self.config.relevance_weight * p.relevance_score +
            self.config.recency_weight * recency
        )
            if p.relevant_score >=self.config.min_relevance:
                score_packets.append(combined_score,p)

        score_packets.sort(key=lambda x: x[0], reverse=True)

    # 5. 贪心选择:按分数从高到低填充,直到达到 token 上限
        selected = system_packets.copy()
        current_tokens = system_tokens

        for score, packet in score_packets:
            if current_tokens + packet.token_count <= available_tokens:
                selected.append(packet)
                current_tokens += packet.token_count
            else:
                # Token 预算已满,停止选择
                break

        print(f"[ContextBuilder] 选择了 {len(selected)} 个信息包,共 {current_tokens} tokens")
        return selected

    def _calculate_relevance(self,content:str,query:str):
        content_words = set(content.lower().split())
        query_words = set(query.lower().split())
        if not query_words:
            return 0.0

    # Jaccard 相似度
        intersection = content_words & query_words
        union = content_words | query_words

    def _calculate_recency(self, timestamp: datetime) -> float:
   
        import math

        age_hours = (datetime.now() - timestamp).total_seconds() / 3600

        # 指数衰减:24小时内保持高分,之后逐渐衰减
        decay_factor = 0.1  # 衰减系数
        recency_score = math.exp(-decay_factor * age_hours / 24)

        return max(0.1, min(1.0, recency_score))  # 限制在 [0.1, 1.0] 范围内

    def _structure(self, selected_packets: list[ContextPacket], user_query: str) -> str:
        system_instructions = []
        evidence = []
        context = []

        for packet in selected_packets:
            packet_type = packet.metadata.get("type", "general")

            if packet_type == "system_instruction":
                system_instructions.append(packet.content)
            elif packet_type in ["rag_result", "knowledge"]:
                evidence.append(packet.content)
            else:
                context.append(packet.content)

            sections = []
            if system_instructions:
                sections.append("Role and policies\n"+"\n".join(system_instructions))

            sections.append(f"[Task]\n{user_query}")

    # [Evidence]
        if evidence:
            sections.append("[Evidence]\n" + "\n---\n".join(evidence))

        # [Context]
        if context:
            sections.append("[Context]\n" + "\n".join(context))

        # [Output]
        sections.append("[Output]\n请基于以上信息,提供准确、有据的回答。")

        return "\n\n".join(sections)

    def _compress(self,context:str,max_token:int):
        token=self._calculate_token(context)
        if (token<=self.max_token):
            return context

        sections=context.split('\n')
        compressed_sessions=[]
        current_token=0

        for section in sections:
            session_tokens=self._calculate_token(section)
            if current_token+session_tokens<=max_token:
                compressed_sessions.append(section)
                current_token+=session_tokens
            else:
                remaining_tokens=max_token-current_token
                truncated = self._truncate_text(section, remaining_tokens)
                compressed_sessions.append(truncated + "\n[... 内容已压缩 ...]")

        compress_text='\n'.join(compressed_sessions)
        final_tokens=self._calculate_token(compress_text)
        return compress_text

    def _trancate_text(self,text:str,max_tokens):
        char_per_token=len(text)/self._calculate_token(text) if self._calculate_token(text) else 4
        max_chars = int(max_tokens * char_per_token)
        return text[:max_chars]

    def _count_tokens(self, text: str) -> int:
    # 简单估算:中文 1 字符 ≈ 1 token,英文 1 单词 ≈ 1.3 tokens
    # 生产环境中应该使用实际的 tokenizer
        chinese_chars = sum(1 for ch in text if '\u4e00' <= ch <= '\u9fff')
        english_words = len([w for w in text.split() if w])

        return int(chinese_chars + english_words * 1.3)


class NoteTool:
    def __init__(self,workspace):
        self.workspace=workspace

    def create_note(self,title,content,note_type,tags:Optional[str]):
        from datetime import datetime
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        note_id = f"note_{timestamp}_{len(self.index)}"

        metadata = {
        "id": note_id,
        "title": title,
        "type": note_type,
        "tags": tags or [],
        "created_at": datetime.now().isoformat(),
        "updated_at": datetime.now().isoformat()
    }
        md_content=self.build_markdown(metadata,content)
        file_path = os.path.join(self.workspace, f"{note_id}.md")
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(md_content)

        metadata['file_path']=file_path
        self.index[note_id]=metadata

        self._save_index()
        return note_id

    def build_markdown(self,metadata:dict,content:str):
        import yaml

    # YAML 前置元数据
        yaml_header = yaml.dump(metadata, allow_unicode=True, sort_keys=False)

    # 组合格式
        return f"---\n{yaml_header}---\n\n{content}"


    def _Read_note(self,note_id):
        if note_id not in self.index:
            raise ValueError(f"No index for {note_id}")

        file_path=self.index[note_id]['file_path']
        with open(file_path, 'r', encoding='utf-8') as f:
            raw_content = f.read()

        # 解析 YAML 元数据和 Markdown 正文
        metadata, content = self._parse_markdown(raw_content)

        return {
            "metadata": metadata,
            "content": content
        }

    def _parse_md(self,raw_content):
        import yaml

    # 查找 YAML 分隔符
        parts = raw_content.split('---\n', 2)
        if len(parts) >= 3:
        # 有 YAML 前置元数据
            yaml_str = parts[1]
            content = parts[2].strip()
            metadata = yaml.safe_load(yaml_str)
        else:
            # 无元数据,全部作为正文
            metadata = {}
            content = raw_content.strip()

        return metadata, content

    def _update_note(
    self,
    note_id: str,
    title: Optional[str] = None,
    content: Optional[str] = None,
    note_type: Optional[str] = None,
    tags: Optional[list[str]] = None
) -> str:
        if note_id not in self.index:
            raise ValueError(f"笔记不存在: {note_id}")

    # 1. 读取现有笔记
        note = self._read_note(note_id)
        metadata = note["metadata"]
        old_content = note["content"]

        # 2. 更新字段
        if title:
            metadata["title"] = title
        if note_type:
            metadata["type"] = note_type
        if tags is not None:
            metadata["tags"] = tags
        if content is not None:
            old_content = content

        # 更新时间戳
        from datetime import datetime
        metadata["updated_at"] = datetime.now().isoformat()

        # 3. 重新构建并保存
        md_content = self._build_markdown(metadata, old_content)
        file_path = metadata["file_path"]

        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(md_content)

        # 4. 更新索引
        self.index[note_id] = metadata
        self._save_index()

        return f"✅ 笔记已更新: {metadata['title']}"

    def _search_notes(
    self,
    query: str,
    limit: int = 10,
    note_type: Optional[str] = None,
    tags: Optional[list[str]] = None
) -> list[Dict]:
        """搜索笔记

        Args:
            query: 搜索关键词
            limit: 返回数量限制
            note_type: 按类型过滤(可选)
            tags: 按标签过滤(可选)

        Returns:
            List[Dict]: 匹配的笔记列表
        """
        results = []
        query_lower = query.lower()

        for note_id, metadata in self.index.items():
            # 类型过滤
            if note_type and metadata.get("type") != note_type:
                continue

            # 标签过滤
            if tags:
                note_tags = set(metadata.get("tags", []))
                if not note_tags.intersection(tags):
                    continue

            # 读取笔记内容
            try:
                note = self._read_note(note_id)
                content = note["content"]
                title = metadata.get("title", "")

                # 在标题和内容中搜索
                if query_lower in title.lower() or query_lower in content.lower():
                    results.append({
                        "note_id": note_id,
                        "title": title,
                        "type": metadata.get("type"),
                        "tags": metadata.get("tags", []),
                        "content": content,
                        "updated_at": metadata.get("updated_at")
                    })
            except Exception as e:
                print(f"[WARNING] 读取笔记 {note_id} 失败: {e}")
                continue

        # 按更新时间排序
        results.sort(key=lambda x: x["updated_at"], reverse=True)

        return results[:limit]

    def _list_notes(
    self,
    note_type: Optional[str] = None,
    tags: Optional[list[str]] = None,
    limit: int = 20
) -> list[Dict]:
        """列出笔记(按更新时间倒序)

        Args:
            note_type: 按类型过滤(可选)
            tags: 按标签过滤(可选)
            limit: 返回数量限制

        Returns:
            List[Dict]: 笔记元数据列表
        """
        results = []

        for note_id, metadata in self.index.items():
            # 类型过滤
            if note_type and metadata.get("type") != note_type:
                continue

            # 标签过滤
            if tags:
                note_tags = set(metadata.get("tags", []))
                if not note_tags.intersection(tags):
                    continue

            results.append(metadata)

        # 按更新时间排序
        results.sort(key=lambda x: x.get("updated_at", ""), reverse=True)

        return results[:limit]

    def _summary(self) -> Dict[str, Any]:
        """生成笔记摘要统计

        Returns:
            Dict: 统计信息
        """
        total_count = len(self.index)

        # 按类型统计
        type_counts = {}
        for metadata in self.index.values():
            note_type = metadata.get("type", "general")
            type_counts[note_type] = type_counts.get(note_type, 0) + 1

        # 最近更新的笔记
        recent_notes = sorted(
            self.index.values(),
            key=lambda x: x.get("updated_at", ""),
            reverse=True
        )[:5]

        return {
            "total_notes": total_count,
            "type_distribution": type_counts,
            "recent_notes": [
                {
                    "id": note["id"],
                    "title": note.get("title", ""),
                    "type": note.get("type"),
                    "updated_at": note.get("updated_at")
                }
                for note in recent_notes
            ]
        }

    def _delete_note(self, note_id: str) -> str:
        """删除笔记

        Args:
            note_id: 笔记ID

        Returns:
            str: 操作结果消息
        """
        if note_id not in self.index:
            raise ValueError(f"笔记不存在: {note_id}")

        # 1. 删除文件
        file_path= self.index[note_id]["file_path"]
        if os.path.exists(file_path):
            os.remove(file_path)

        # 2. 从索引中移除
        title = self.index[note_id].get("title", note_id)
        del self.index[note_id]
        self._save_index()

        return f"✅ 笔记已删除: {title}"
notes = NoteTool(workspace="./project_notes")

note_id = notes.run({
    "action": "create",
    "title": "重构项目 - 第一阶段",
    "content": """## 完成情况
已完成数据模型层的重构,测试覆盖率达到85%。

## 下一步
重构业务逻辑层""",
    "note_type": "task_state",
    "tags": ["refactoring", "phase1"]
})

print(f"✅ 笔记创建成功,ID: {note_id}")




        
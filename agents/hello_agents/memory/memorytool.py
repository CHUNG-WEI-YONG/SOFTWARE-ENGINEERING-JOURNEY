from datetime import datetime
from typing import Optional, List, Tuple
from hello_agents.tools import Tool
from memory_manager import MemoryManager
class MemoryTool(Tool):
    def __init__(self, user_id:int,memory_config:MemoryConfig=None, memory_types:List[str]=None):
        super().__init__(name="memory", description="记忆工具 - 可以存储和检索对话历史、知识和经验")

        self.memory_config=memory_config or MemoryConfig()
        self.memory_types=memory_types or ["working","episodic","semantic","perceptual"]

        self.memory_manager = MemoryManager(
            config=self.memory_config,
            user_id=user_id,
            enable_working="working" in self.memory_types,
            enable_episodic="episodic" in self.memory_types,
            enable_semantic="semantic" in self.memory_types,
            enable_perceptual="perceptual" in self.memory_types
        )

    def run(self,action,**kwargs):
        return self.execute(action,**kwargs)

    
    def execute(self,action:str,**kwargs):
        if action=="add":
            return self._add_memory(**kwargs)
        elif action=="search":
            return self._search_memory(**kwargs)
        elif action=="summary":
            return self._summary_memory(**kwargs)
    

    def _add_memory(
            self,
            content:str,
            importance:float=0.5,
            memory_type:str="working",
            file_path:str=None,
            modality:str=None,
            **metadata):
        try:
            if self.current_session_id is None:
                self.current_session_id = f"session_{datetime.now().strftime('%Y%m%d_%H%M%S')}"

            if memory_type=="perceptual" and file_path:
                inferred=modality or self._infer_modality(file_path)
                metadata.setdefault("modality",inferred)
                metadata.setdefault("raw_data",file_path)

            metadata.update({
                "session_id":self.current_session_id,
                "timestamp":datetime.now().isoformat(),
            })

            memory_id=self.memory_manager.add_memory(
                content=content,
                memory_type=memory_type,
                importance=importance,
                metadata=metadata,
                auto_classify=False
            )

            return f"✅ 记忆已添加 (ID: {memory_id[:8]}...)"

        except Exception as e:
            return f"❌ 添加记忆失败: {str(e)}"

    def _search_memory(self,query:str,limit:int=5,memory_types:List[str]=None,memory_type:str=None,min_importance:int=0.1):
        try:
            if memory_type and not memory_types:
                memory_types=[memory_type]

            result=self.memory_manager.retrieve_memories(
                query=query,
                memory_types=memory_types,
                limit=limit,
                importance=min_importance,
            )
            if not result:
                return f"🔍 未找到与 '{query}' 相关的记忆"

            formatted_result=[]
            formatted_result.append(f"🔍 找到 {len(result)} 条相关记忆:")

            for i , memory in enumerate(result,1):
                memory_type_label = {
                "working": "工作记忆",
                "episodic": "情景记忆", 
                "semantic": "语义记忆",
                "perceptual": "感知记忆"
            }.get(memory.memory_type, memory.memory_type)

                content_preview = memory.content[:80] + "..." if len(memory.content) > 80 else memory.content
                formatted_result.append(f"{i}. [{memory_type_label}] {content_preview} (重要性: {memory.importance:.2f})" )
            return "\n".join(formatted_result)
        

        except Exception as e:
            return f"❌ 添加记忆失败: {str(e)}"


    def _delete_memory(self,strategy:str="Importance_based",threshold:float=0.1,max_forget_day:int=30):
        try:
            count=self.memory_manager.forgot_memory(
                threshold=threshold,
                strategy=strategy,
                max_age_days=max_forget_day
            )
            return f"🧹 已遗忘 {count} 条记忆（策略: {strategy})"
        except Exception as e:
            return  f"❌ 遗忘记忆失败: {str(e)}"

    def _consolidate(self,from_type:str="Working",to_type:str="episodic",importance_threshold:float=0.5):
        try:
            count=self.memory_manager.consolidate(
                from_type=from_type,
                to_type=to_type,
                importance_threshold=importance_threshold
            )
            return f"🔄 已整合 {count} 条记忆为长期记忆（{from_type} → {to_type}，阈值={importance_threshold}）"
        except Exception as e:
            return f"❌ 整合记忆失败: {str(e)}"
    

memory_tool=MemoryTool()

# 1. 工作记忆 - 临时信息，容量有限
memory_tool.run("add",
    content="用户刚才问了关于Python函数的问题",
    memory_type="working",
    importance=0.6
)

# 2. 情景记忆 - 具体事件和经历
memory_tool.run("add",
    content="2024年3月15日，用户张三完成了第一个Python项目",
    memory_type="episodic",
    importance=0.8,
    event_type="milestone",
    location="在线学习平台"
)

# 3. 语义记忆 - 抽象知识和概念
memory_tool.run("add",
    content="Python是一种解释型、面向对象的编程语言",
    memory_type="semantic",
    importance=0.9,
    knowledge_type="factual"
)

# 4. 感知记忆 - 多模态信息
memory_tool.run("add",
    content="用户上传了一张Python代码截图，包含函数定义",
    memory_type="perceptual",
    importance=0.7,
    modality="image",
    file_path="./uploads/code_screenshot.png"
)

# 基础搜索
result = memory_tool.execute("search", query="Python编程", limit=5)

# 指定记忆类型搜索
result = memory_tool.execute("search",
    query="学习进度",
    memory_type="episodic",
    limit=3
)

# 多类型搜索
result = memory_tool.execute("search",
    query="函数定义",
    memory_types=["semantic", "episodic"],
    min_importance=0.5
)
from hello_agents.tools import Tool
from typing import Optional, List, Tuple

class MemoryManager:
    """记忆管理器 - 统一的记忆操作接口"""

    def __init__(
        self,
        config: Optional[MemoryConfig] = None,
        user_id: str = "default_user",
        enable_working: bool = True,
        enable_episodic: bool = True,
        enable_semantic: bool = True,
        enable_perceptual: bool = False
    ):
        self.config=config or UserConfig()
        self.user_id=user_id

        self.store=MmeoryStore(self.config)
        self.retriever=MemoryRetriever(self.store,self.config)

        self.memory_types = {}

        if enable_working:
            self.memory_types['working'] = WorkingMemory(self.config, self.store)

        if enable_episodic:
            self.memory_types['episodic'] = EpisodicMemory(self.config, self.store)

        if enable_semantic:
            self.memory_types['semantic'] = SemanticMemory(self.config, self.store)

        if enable_perceptual:
            self.memory_types['perceptual'] = PerceptualMemory(self.config, self.store)



class WorkingMemory:
    def __init__(self,config:MemoryConfig):
        self.max_life=config.max_ttb_life or 60
        self.capacity=config.working_memory_capacity or 50
        self.memory=[]

    def add(self,memoryItem:MemoryItem):
        self._expired_memory_clear()
        if len(self.memory)>=self.capacity:
            self._remove_lowest_priority_memory()
        self.memory.append(memoryItem)
        return memoryItem.id

    def retrieve(self,query:str,limit:int=5,**kwargs)->List[MemoryItem]:
        self._expire_old_memories()
        
        # 尝试TF-IDF向量检索
        vector_scores = self._try_tfidf_search(query)

        scored_memories = []
        for memory in self.memories:
            vector_score = vector_scores.get(memory.id, 0.0)
            keyword_score = self._calculate_keyword_score(query, memory.content)
            
            # 混合评分
            base_relevance = vector_score * 0.7 + keyword_score * 0.3 if vector_score > 0 else keyword_score
            time_decay = self._calculate_time_decay(memory.timestamp)
            importance_weight = 0.8 + (memory.importance * 0.4)
            
            final_score = base_relevance * time_decay * importance_weight
            if final_score > 0:
                scored_memories.append((final_score, memory))
        
        scored_memories.sort(key=lambda x: x[0], reverse=True)
        return [memory for _, memory in scored_memories[:limit]]


class EpisodicMemory:
    def __init__(self,config:MessageConfig):
        self.data_store=SQLiteDocumentStore(config.data_path)
        self.vector_store = QdrantVectorStore(config.qdrant_url, config.qdrant_api_key)
        self.embedder = create_embedding_model_with_fallback()
        self.sessions = {}  # 会话索引

    def add(self,memory:MemoryItem):
        episode=Episode(
            episode_id=memory_item.id,
            session_id=memory_item.metadata.get("session_id", "default"),
            timestamp=memory_item.timestamp,
            content=memory_item.content,
            context=memory_item.metadata
        )
        session_id=episode.session_id
        if session_id not in self.sessions:
            self.sessions[session_id]=[]
        self.sessions[session_id].append(episode.episode_id)
        self._persist_episode(episode)
        return memory.id

    def retrieve(self,query:str,limit:int=5,**kwargs):
        candidate_ids=self._structure_filter(**kwargs)
        hits = self._vector_search(query, limit * 5, kwargs.get("user_id"))

        results=[]
        for hit in hits:
            if self._should_include(hit, candidate_ids, kwargs):
                score = self._calculate_episode_score(hit)
                memory_item = self._create_memory_item(hit)
                results.append((score, memory_item))
        results.sort(key=lambda x:x[0],reverse=True)
        return results[item for _ ,item in results[:limit] ]

    def _calculate_episode_score(self,hit):
        vec_score=float(hit.get("score",0.0))
        recency_score = self._calculate_recency(hit["metadata"]["timestamp"])
        importance = hit["metadata"].get("importance", 0.5)
        
        # 评分公式：(向量相似度 × 0.8 + 时间近因性 × 0.2) × 重要性权重
        base_relevance = vec_score * 0.8 + recency_score * 0.2
        importance_weight = 0.8 + (importance * 0.4)
        
        return base_relevance * importance_weight

class SemanticMemory(BaseMemory):
    def __init__(self,config:MemoryConfig,storage_backend=None):
        super().init(config,storage_backend)
        self.embedding_model=get_text_embedder()
        self.vector_store=QdrantConnectionManager.get_instance(**qdrant_config)
        self.graph_store=Neo4jGraphStore(**neo4j_config)

        self.entities:Dict[str,Entity]={}
        self.relations:List[Relation]=[]
        self.nlp=self._init_nlp()

    def add(self,memory_item:MemoryItem):
        embedding=self.embedding_model.encode(memory_item.content)

        entities=self._extract_entities(memory_item.content)
        relations=self._extract_relation(memory_item.content,entities)

        for entity in entities:
            self._add_entity_to_graph(entity)
        for relation in relations:
            self._add_relation_to_graph(relation)

        metadata={
            "memory_id":memory_item.id,
            "entities":[e.entity_id for e in entities],
            "entity_count":len(entities),
            "relation_count":len(relations)
        }

        self.vector_store.add_vectors(
            vectors=[embedding.tolist()],
            metadata=[metadata],
            ids=[memory_item.id]

        )

    def retrieve(self,query,limit:int=5,**kwargs):
        vector_results=self._vector_search(query,limit*2,user_id)
        graph_results = self._graph_search(query, limit * 2, user_id)
    
    # 3. 混合排序
        combined_results = self._combine_and_rank_results(
            vector_results, graph_results, query, limit
        )
    
        return combined_results[:limit]

    def _combine_and_rank_results(self, vector_results, graph_results, query, limit):
        combined = {}
        
        # 合并向量和图检索结果
        for result in vector_results:
            combined[result["memory_id"]] = {
                **result,
                "vector_score": result.get("score", 0.0),
                "graph_score": 0.0
            }
        
        for result in graph_results:
            memory_id = result["memory_id"]
            if memory_id in combined:
                combined[memory_id]["graph_score"] = result.get("similarity", 0.0)
            else:
                combined[memory_id] = {
                    **result,
                    "vector_score": 0.0,
                    "graph_score": result.get("similarity", 0.0)
                }
        
        # 计算混合分数
        for memory_id, result in combined.items():
            vector_score = result["vector_score"]
            graph_score = result["graph_score"]
            importance = result.get("importance", 0.5)
            
            # 基础相似度得分
            base_relevance = vector_score * 0.7 + graph_score * 0.3
            
            # 重要性权重 [0.8, 1.2]
            importance_weight = 0.8 + (importance * 0.4)
            
            # 最终得分：相似度 * 重要性权重
            combined_score = base_relevance * importance_weight
            result["combined_score"] = combined_score
        
        # 排序并返回
        sorted_results = sorted(
            combined.values(),
            key=lambda x: x["combined_score"],
            reverse=True
        )
        
        return sorted_results[:limit]

class PerceptualMemory(BaseMemory):
    def __init__(self,config:MemoryConfig,storage_backend=None):
        super().__init__(config,storage_backend)
        self.text_embedder=init_text_embedder()
        self.clip_model=self._init_clip_model()
        self.clap_model=self._init_clap_model()

        self.vector_stores = {
            "text": QdrantConnectionManager.get_instance(
                collection_name="perceptual_text",
                vector_size=self.vector_dim
            ),
            "image": QdrantConnectionManager.get_instance(
                collection_name="perceptual_image", 
                vector_size=self._image_dim
            ),
            "audio": QdrantConnectionManager.get_instance(
                collection_name="perceptual_audio",
                vector_size=self._audio_dim
            )
        }


    def retrieve(self,query:str,limit:int=5,**kwargs):
        user_id=kwargs.get("user_id")
        target_modality=kwargs.get("target_modality")
        query_modality=kwargs.get("query_modality",target_modality or "text")
        try:
            query_vector=self.encode_data(query,query_modality)
            store=self.get_vector_store_for_modality(target_modality,query_modality)
            where={"memory_type":"perceptual"}
            if user_id:
                where["user_id"] = user_id
            if target_modality:
                where["modality"] = target_modality
            
            hits = store.search_similar(
                query_vector=query_vector,
                limit=max(limit * 5, 20),
                where=where
            )
        except Exception:
            hits=[]
        results=[]

        for hit in hits:
            vector_score = float(hit.get("score", 0.0))
        recency_score = self._calculate_recency_score(hit["metadata"]["timestamp"])
        importance = hit["metadata"].get("importance", 0.5)
        
        # 评分算法
        base_relevance = vector_score * 0.8 + recency_score * 0.2
        importance_weight = 0.8 + (importance * 0.4)
        combined_score = base_relevance * importance_weight
        
        results.append((combined_score, self._create_memory_item(hit)))
    
        results.sort(key=lambda x: x[0], reverse=True)
        return [item for _, item in results[:limit]]

    def _calculate_recency_score(self, timestamp: str) -> float:
        """计算时间近因性得分"""
        try:
            memory_time = datetime.fromisoformat(timestamp)
            current_time = datetime.now()
            age_hours = (current_time - memory_time).total_seconds() / 3600
            
            # 指数衰减：24小时内保持高分，之后逐渐衰减
            decay_factor = 0.1  # 衰减系数
            recency_score = math.exp(-decay_factor * age_hours / 24)
            
            return max(0.1, recency_score)  # 最低保持0.1的基础分数
        except Exception:
            return 0.5  # 默认中等分数
        
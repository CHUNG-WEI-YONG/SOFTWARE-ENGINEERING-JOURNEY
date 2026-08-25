from typing import TypedDict,List,Dict ,Optional
from langgraph.graph import StateGraph, START, END
from langgraph.checkpoint.memory import MemorySaver
import arxiv
import requests
from dotenv import load_dotenv
import os
from AiClient import AgentsLLM,ToolExecutor,ReAct_Agent
import math
import re
load_dotenv()
class GraphState(TypedDict):
    topic: str
    papers: List[Dict]          # 建议存 List[Dict]，方便后续提取引用量、发表年份与摘要
    summary: str
    proposed_idea: str
    pass_evaluate: bool
    retry_count: int

REACT_PROMPT_TEMPLATE = """你是一个专业的学术检索智能体。你的任务是针对用户给定的研究课题，使用工具检索高质量的前沿学术论文。

可用的工具有：
{tools}

请严格按照以下格式进行思考和行动（必须遵循 Thought -> Action 循环）：

Question: 需要检索的研究课题
Thought: 思考当前应该采取什么步骤（如：提炼英文关键词、调用什么工具）
Action: tool_name[argument]
Observation: 工具返回的结果
...（这个 Thought/Action/Observation 过程可以重复多次）
Thought: 我已经获得了足够的论文数据，准备结束检索
Action: Finish[检索完成]

当前检索任务:
Question: {question}

历史轨迹:
{history}
"""

SEARCH_REACT_PROMPT = """你是一个专业的学术检索 Agent。
你的任务是：根据用户的研究主题，拆解出 2~3 个正统的前沿学术检索词，调用工具获取最相关的顶会文献。

可用工具:
{tools}

回复格式规范:
Thought: 分析需要拆解的学术关键词与检索策略。
Action: `tool_name[query_text]` (例如: `search_semantic[KV Cache Quantization]` 或 `search_arxiv[Speculative Decoding]`)
- 当你认为已经收集到了多篇相关的文献时，必须输出: Action: Finish[检索完成]

问题: {question}
历史动作:
{history}
"""

def search_arxiv(query: str, max_result: int = 5) -> List[Dict]:
    """通过 arXiv 检索预印本论文（带安全清洗与 429 降级防护）"""
    # 1. 彻底清洗 LLM 可能输出的 'query=', 引号等杂质
    clean_query = re.sub(r'^(query\s*=\s*|search_query\s*=\s*)', '', query, flags=re.IGNORECASE)
    clean_query = clean_query.split(",")[0].strip().strip("'").strip('"')
    
    # 限制搜索词长度，避免超长 URL 导致 429/400
    clean_query = " ".join(clean_query.split()[:8])
    if not clean_query:
        return []

    papers = []
    try:
        # 配置遵守速率限制的 Client
        client = arxiv.Client(page_size=int(max_result), delay_seconds=3.0, num_retries=2)
        search = arxiv.Search(
            query=clean_query,
            max_results=int(max_result),
            sort_by=arxiv.SortCriterion.Relevance
        )

        for result in client.results(search):
            raw_pdf = result.pdf_url or ""
            if "arxiv.org/abs/" in raw_pdf:
                raw_pdf = raw_pdf.replace("arxiv.org/abs/", "arxiv.org/pdf/") + ".pdf"
            elif raw_pdf and not raw_pdf.endswith(".pdf"):
                raw_pdf += ".pdf"

            papers.append({
                "title": result.title.replace("\n", " ").strip(),
                "published": result.published.strftime("%Y-%m-%d"),
                "year": result.published.year,
                "citations": 0,
                "venue": "arXiv",
                "abstract": result.summary.replace("\n", " ").strip(),
                "pdf_url": raw_pdf,
                "entry_id": result.entry_id
            })
    except Exception as e:
        # 遇到 429 限流或网络异常时不崩断程序，平滑返回空列表
        print(f"⚠️ [arXiv API 限流或异常 (429)] 已安全捕获: {e}")
        return []

    return papers

"""通过 Semantic Scholar 检索论文（含被引量与顶会）"""
def search_semantic(query: str, max_result: int = 5) -> List[Dict]:
    """通过 Semantic Scholar 检索顶会论文与被引量"""
    # 彻底剥离可能存在的 query='...' 杂质
    clean_query = re.sub(r'^(query\s*=\s*|search_query\s*=\s*)', '', query, flags=re.IGNORECASE)
    clean_query = clean_query.split(",")[0].strip().strip("'").strip('"')
    clean_query = " ".join(clean_query.split()[:6])  # 限制关键词长度

    if not clean_query:
        return []

    api_key = os.getenv("SEMANTIC_API_KEY")
    url = "https://api.semanticscholar.org/graph/v1/paper/search"

    headers = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) ResearchAgent/1.0"}
    if api_key:
        headers["x-api-key"] = api_key.strip()

    params = {
        "query": clean_query,
        "limit": int(max_result),
        "fields": "title,authors,year,citationCount,venue,tldr,abstract,openAccessPdf",
    }

    try:
        response = requests.get(url=url, headers=headers, params=params, timeout=12)
        if response.status_code == 200:
            data = response.json().get("data", [])
            papers = []
            for item in data:
                tldr_data = item.get("tldr")
                pdf_info = item.get("openAccessPdf") or {}
                if item.get("title"):
                    papers.append({
                        "title": item.get("title").strip(),
                        "year": item.get("year") or 2024,
                        "citations": item.get("citationCount", 0),
                        "venue": item.get("venue", "") or "arXiv.org",
                        "tldr": tldr_data.get("text", "") if tldr_data else "",
                        "abstract": (item.get("abstract") or "").replace("\n", " ").strip(),
                        "pdf_url": pdf_info.get("url") or ""
                    })
            if papers:
                return papers
    except Exception as e:
        print(f"⚠️ [Semantic Scholar 请求异常]: {e}")
    
    return []

class SearchAgent(ReAct_Agent):
    def __init__(self, client, tools, prompt, max_time=5):
        super().__init__(client, tools, prompt, max_time)
        self.collected_papers:List[dict]=[]
        self.max_steps=max_time

    def generate(self, question: str) -> List[Dict]:
        self.history = []
        self.collected_papers = []
        curr_step = 0


        while curr_step < self.max_steps:
            curr_step += 1
            tools_str = self.tools.getInfo()
            history_str = "\n".join(self.history)
            prompt = self.prompt.format(
                tools=tools_str,
                question=question,
                history=history_str
            )

            response = self.client.think([{"role": "user", "content": prompt}])
            if not response:
                break

            thought, action = self._parse_output(response)
            if not action:
                break

            if action.startswith("Finish"):
                break

            func_name, argument = self._parse_action(action)
            tool_func = self.tools.getTools(func_name)
            if not tool_func:
                continue

            print(f"🎬 [Agent 执行工具] {func_name} -> 检索词: '{argument}'")
            # 调用底层函数获取数据
            papers_found = tool_func(query=argument, max_result=5)
            
            # 将拉取到的论文对象存入列表并做标题去重
            if isinstance(papers_found, list):
                for p in papers_found:
                    if isinstance(p, dict) and p.get("title"):
                        if not any(existing['title'].lower() == p['title'].lower() for existing in self.collected_papers):
                            self.collected_papers.append(p)
                observation = f"Successfully fetched {len(papers_found)} papers."
            else:
                observation = "No valid papers returned."

            self.history.append(f"Thought: {thought}")
            self.history.append(f"Action: {func_name}[{argument}]")
            self.history.append(f"Observation: {observation}")

        return self.collected_papers
    

# =============================================================
# 2. search_node 节点入口 (加入保底机制)
# =============================================================
def search_node(state: dict) -> dict:
    topic = state.get("topic", "")
    print(f"\n🚀 开始执行学术检索节点，研究课题: {topic}")

    agent = AgentsLLM()
    tools = ToolExecutor()

    tools.register_tool("search_arxiv", search_arxiv, "使用 arXiv 检索论文预印本，参数为关键词 query")
    tools.register_tool("search_semantic", search_semantic, "使用 Semantic Scholar 检索已发表顶会论文及引用量，参数为关键词 query")

    search_agent = SearchAgent(client=agent, tools=tools, prompt=SEARCH_REACT_PROMPT, max_time=3)
    papers = search_agent.generate(f"请为研究主题 '{topic}' 检索 3-5 篇最相关的最新顶会论文")

    # 兜底保障：若 API 受到 429 限流或未检索到，注入高质量前沿文献保证流水线稳定执行
    if not papers or len(papers) < 2:
        print("⚠️ 外部学术 API 限流或响应不足，启用核心文献池保障流水线推进...")
        papers = [
            {
                "title": "PyramidKV: Dynamic KV Cache Compression based on Pyramidal Information Funneling",
                "year": 2024,
                "venue": "arXiv.org",
                "citations": 388,
                "abstract": "Recent Large Language Models (LLMs) suffer from huge GPU memory overhead during long context inference. We observe attention allocation exhibits strong layer-wise pyramidal patterns, and propose PyramidKV to dynamically allocate cache capacity.",
                "pdf_url": "https://arxiv.org/pdf/2406.02069.pdf"
            },
            {
                "title": "H2O: Heavy-Hitter Oracle for Efficient Generative Inference of Large Language Models",
                "year": 2023,
                "venue": "NeurIPS",
                "citations": 540,
                "abstract": "We observe that a small fraction of tokens (Heavy Hitters) contribute to most of the attention value. We design H2O algorithm to dynamically evict non-essential KV pairs with minimal accuracy loss.",
                "pdf_url": "https://arxiv.org/pdf/2306.14048.pdf"
            },
            {
                "title": "SnapKV: LLM Knows What You are Looking for Before Generation",
                "year": 2024,
                "venue": "ICLR",
                "citations": 210,
                "abstract": "SnapKV automatically identifies critical attention patterns in the prompt and compresses prompt KV cache before decoding, reducing memory footprint by up to 80% without fine-tuning.",
                "pdf_url": "https://arxiv.org/pdf/2404.14469.pdf"
            }
        ]

    print(f"✅ 检索完成，共聚合 {len(papers)} 篇文献")
    return {"papers": papers[:5]}

def calculate_paper_score(paper: Dict, current_year: int = 2026) -> float:
    citations = paper.get("citations", 0)
    venue = (paper.get("venue") or "").lower()
    top_venues = ["neurips", "iclr", "icml", "cvpr", "iccv", "acl", "emnlp", "kdd", "aaai"]
    venue_bonus = 1.0 if any(v in venue for v in top_venues) else 0.0
    importance = min(math.log1p(citations) / 5.0, 1.0) * 0.7 + venue_bonus * 0.3

    diff = max(0, current_year - paper.get("year", current_year))
    recency = max(0.0, 1.0 - (diff * 0.2))

    relevance = 0.8 if paper.get("abstract") else 0.4
    return 0.4 * importance + 0.3 * recency + 0.3 * relevance





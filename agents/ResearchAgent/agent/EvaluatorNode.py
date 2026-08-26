from typing import TypedDict, List, Dict
from AiClient import AgentsLLM
import os
import re
import json
import requests
from typing import TypedDict, List, Dict, Tuple

class EvaluatorResult(TypedDict):
    summary: str
    proposed_idea: str
    pytorch_code: str
    novelty_check_report: str
    pass_evaluate: bool
    retry_count: int


# =============================================================
# 2. 反向检索查重工具 (Semantic Scholar Reverse Search)
# =============================================================
def reverse_novelty_search(keywords: str, limit: int = 4) -> List[Dict]:
    """
    使用 Semantic Scholar 针对拟定创新点的核心关键词进行最新文献反向查重
    """
    api_key = os.getenv("SEMANTIC_API_KEY")
    url = os.getenv("SEMANTIC_URL") or "https://api.semanticscholar.org/graph/v1/paper/search"

    headers = {"User-Agent": "ResearchAgent/NoveltyChecker"}
    if api_key:
        headers["x-api-key"] = api_key.strip()

    params = {
        "query": keywords,
        "limit": limit,
        "fields": "title,year,venue,citationCount,abstract,openAccessPdf",
        "year": "2024-"  # 聚焦近 2 年最新成果
    }

    try:
        res = requests.get(url, headers=headers, params=params, timeout=12)
        if res.status_code == 200:
            data = res.json().get("data", [])
            return [
                {
                    "title": item.get("title", ""),
                    "year": item.get("year", ""),
                    "venue": item.get("venue", ""),
                    "citations": item.get("citationCount", 0),
                    "abstract": (item.get("abstract") or "")[:200]
                }
                for item in data if item.get("title")
            ]
    except Exception as e:
        print(f"[!] 反向查重 API 异常: {e}")
    return []


# =============================================================
# 3. 提示词工程 (蓝方出题 + PyTorch 建模)
# =============================================================
BLUE_TEAM_PROPOSAL_PROMPT = """你是一位顶会（NeurIPS / ICLR / ICML）资深算法专家。
请仔细分析以下 5 篇前沿文献的深度精读矩阵，提炼痛点并设计一份【极具可行性与创新性】的顶会选题方案。

精读文献档案：
{summaries_content}

请严格按以下 JSON 结构输出（不要输出任何多余的 Markdown 标记或外部文本）：
```json
{{
  "literature_synthesis": "横向对比现有方案在显存、计算开销、免训练性与长文本注意力上的共性瓶颈",
  "paper_title": "英文建议论文标题（体现算法创新）",
  "search_keywords_for_check": "用于在 Semantic Scholar 查重的 3~4 个核心技术专有名词（空格分隔）",
  "motivation": "直击哪个尚未解决的深层缺陷与痛点",
  "methodology": "详细叙述核心算法机制、数据结构设计与数学建模公式",
  "baselines": ["对比基线 1", "对比基线 2", "对比基线 3"],
  "benchmarks": ["评测数据集与任务 1", "指标 (如 Perplexity, LongBench, Throughput, Peak VRAM)"],
  "pytorch_code": "完整的 PyTorch 核心算法模块/算子实现代码，必须包含 __init__ 和 forward 方法，且对每个 Tensor 操作附带详细的维度注释 (例如: # [Batch, Num_Heads, Seq_Len, Head_Dim])"
}}"""
EVALUATOR_PROMPT_TEMPLATE = """你是一位国际顶级会议（NeurIPS / ICLR / ICML / ACL）的资深领域主席（Senior Area Chair）。

以下是 5 位审稿专家对课题「{topic}」最新核心文献进行深度正文剖析后提交的技术档案：
{summaries_content}

请执行两项核心任务：

==================================================
任务一：综合文献综述与深层瓶颈提炼 (Deep Research Gaps)
==================================================
1. 梳理各方案的共性技术局限（如：注意力分布畸变、长文本检索精度骤降、硬件不友好等）。
2. 指出现有基准测试（Benchmarks）存在的评测盲区。

==================================================
任务二：设计具备顶会竞争力的创新选题方案 (Proposal)
==================================================
严格要求：聚焦于算法创新、免训练优化或架构改进，适合单卡/少量 GPU（如单张 RTX 4090 或 A100）验证，坚决杜绝海量算力预训练题目。

请按以下格式输出完整提案：
- **建议论文标题 (English Title)**: 
- **研究动机 (Motivation)**: （直击上述哪一个未解决的局限）
- **核心方法设计 (Methodology)**: （详细描述算法机制、数据结构设计与核心数学公式思想）
- **基线模型与对比方法 (Baselines)**: （至少列出 3 个对比算法）
- **评测指标与数据集 (Evaluation & Benchmarks)**: （具体评测任务，如 LongBench、RULER、Throughput、Latency）
- **预期学术贡献 (Key Contributions)**: 

==================================================
任务三：同行评审决策 (Decision)
==================================================
在最后一行给出结论：
若方案完备且创新性强，输出: `RESULT: PASS`
若存在严重算力依赖或创新不足，输出: `RESULT: FAIL [具体缺陷说明]`"""


def evaluate_node(state:dict)->dict:
    topic=state.get("topic","")
    summaries=state.get("summaries",[])
    retry_count=state.get("retry_count",0)

    print(f"\n==================================================")
    print(f"⚖️ [Evaluator Node 启动] 正在集中审查 {len(summaries)} 份精读报告...")
    print(f"==================================================")

    formatted_summaries=[]
    for idx, s in enumerate(summaries, 1):
        block = f"""
【文献 {idx}】: 《{s.get('title')}》 ({s.get('year')} / {s.get('venue')})
- 引用量: {s.get('citations', 0)}
- 核心算法机制: {s.get('core_mechanism')}
- 权衡与代价: {s.get('tradeoffs')}
- 正文承认局限: {s.get('limitations')}
"""
        formatted_summaries.append(block)
    summaries_text="\n".join(formatted_summaries)
    prompt = EVALUATOR_PROMPT_TEMPLATE.format(
        topic=topic,
        summaries_content=summaries_text
    )

    agent = AgentsLLM()
    review_output = agent.think([{"role": "user", "content": prompt}], temperature=0.2)
    is_passed = "RESULT: PASS" in review_output
    print(f"[*] Evaluator 最终评审决策: {'✅ 通过 (PASS)' if is_passed else '❌ 未通过 (FAIL)'} (迭代轮次: {retry_count + 1})")

    return {
        "summary": review_output,
        "proposed_idea": review_output,
        "pass_evaluate": is_passed,
        "retry_count": retry_count + 1
    }

if __name__ == "__main__":
    # 模拟 Worker 总结后的假数据进行测试
    mock_summaries = [
        {
            "title": "PyramidKV: Dynamic KV Cache Compression",
            "year": 2024,
            "venue": "arXiv.org",
            "citations": 388,
            "core_mechanism": "利用注意力金字塔层级漏斗特性，底层保留较少 Cache，顶层保留较多 Cache。",
            "tradeoffs": "降低了显存，但在跨层交互极强的任务中会损失 1~2% 精度。",
            "limitations": "对超长长文本（>64k）的 Needle-in-a-Haystack 任务表现出召回率急剧衰减。"
        },
        {
            "title": "KVzip: Query-Agnostic Context Reconstruction",
            "year": 2025,
            "venue": "NeurIPS",
            "citations": 72,
            "core_mechanism": "通过低秩投影矩阵重构 KV 向量上下文表征。",
            "tradeoffs": "无需依赖 Query，但投影矩阵的解码计算带来了轻微的时延开销。",
            "limitations": "在量化（如 INT4/FP4）状态下重构误差会放大，导致困惑度激增。"
        }
    ]

    test_state = {
        "topic": "KV Cache Compression in LLM",
        "summaries": mock_summaries,
        "retry_count": 0
    }

    result = evaluate_node(test_state)
    print("\n📦 【Evaluator 产出预览】:")
    print(result["proposed_idea"][:600] + "...\n")
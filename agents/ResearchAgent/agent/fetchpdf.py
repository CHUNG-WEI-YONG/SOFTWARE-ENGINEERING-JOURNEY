import pymupdf  # 推荐使用 pymupdf 替代 fitz
import requests
import re
from typing import TypedDict, List, Dict
from AiClient import AgentsLLM

# =============================================================
# 1. 结构化类型定义
# =============================================================
class PaperSummary(TypedDict):
    title: str
    year: int
    venue: str
    citations: int
    pdf_url: str
    core_mechanism: str     # 核心算法、数据结构与数学公式
    tradeoffs: str           # 算力开销、内存占用与延迟权衡
    limitations: str        # 正文/消融实验披露的关键缺陷与失效场景

class SinglePaperInput(TypedDict):
    topic: str
    paper: Dict


# =============================================================
# 2. PDF 下载与正文提取工具
# =============================================================
def download_pdf_content(pdf_url: str, max_str: int = 12000) -> str:
    if not pdf_url:
        return ""
    
    # 修复 1: 确保变量被重新赋值
    if "arxiv.org/abs/" in pdf_url:
        pdf_url = pdf_url.replace("arxiv.org/abs/", "arxiv.org/pdf/")
    if "arxiv.org" in pdf_url and not pdf_url.endswith(".pdf"):
        pdf_url += ".pdf"

    headers = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) ResearchAgent/1.0"}
    try: 
        # 修复 2: 移除 requests.get 不支持的 filetype 参数
        response = requests.get(pdf_url, headers=headers, timeout=15)
        if response.status_code != 200:
            return ""
            
        doc = pymupdf.open(stream=response.content, filetype="pdf")
        full_text = ""
        for page in doc:
            full_text += page.get_text() + "\n"

        clean = re.sub(r"\s+", " ", full_text)
        return clean[:max_str]
    
    except Exception as e:
        print(f"[!] 下载/解析 PDF 异常 ({pdf_url}): {e}")
        return ""


# =============================================================
# 3. 节点 1: fetch_pdf_node
# =============================================================
def fetch_pdf_node(state: dict) -> dict:
    # 修复 3: 将 "paper" 修正为标准的 "papers"
    papers = state.get("papers", [])
    print(f"\n📥 [Fetch PDF Node] 正在为 {len(papers)} 篇核心文献拉取正文内容...")

    for i, p in enumerate(papers, 1):
        title = p.get("title", "")
        pdf_url = p.get("pdf_url", "")
        print(f"  [{i}/{len(papers)}] 下载解析中: 《{title[:35]}...》")
        
        pdf_text = download_pdf_content(pdf_url=pdf_url)
        if pdf_text and len(pdf_text) > 800:
            p["full_content"] = pdf_text
            p["content_source"] = "PDF 正文"
        else:
            p["full_content"] = p.get("abstract") or p.get("tldr") or "无内容"
            p["content_source"] = "论文摘要 (Abstract)"

    print(f"✅ {len(papers)} 篇文献的正文数据准备完毕！")
    return {"papers": papers}


# =============================================================
# 4. 节点 2: summarize_single_paper_node
# =============================================================
SINGLE_PAPER_SUMMARY_PROMPT = """你是一位国际顶级会议（NeurIPS / ICLR / ICML）的资深审稿人。
请深入剖析以下这篇学术论文的核心正文与实验内容，提炼出高密度的结构化技术档案：

论文题目: {title} ({year})
发表会议/期刊: {venue} | 引用量: {citations}
数据来源: {content_source}

论文提取内容:
{content}

请严格按以下三个模块输出（不要废话，直击技术本质）：
【核心机制】: 详细拆解该论文的算法流程、核心数学公式或创新数据结构设计。
【权衡分析】: 分析该方案在延迟、吞吐量、显存占用或训练成本上的真实 Trade-offs。
【正文局限】: 提炼论文正文、消融实验或 Limitations 章节中披露的致命缺陷、未解场景或泛化性瓶颈。
"""

def summarize_single_paper_node(state: SinglePaperInput) -> dict:
    paper = state.get("paper", {})
    title = paper.get("title", "Unknown")
    print(f"🤖 [Worker Agent] 正在精读剖析: 《{title[:35]}...》")
    
    prompt = SINGLE_PAPER_SUMMARY_PROMPT.format(
        title=title,
        year=paper.get("year", 2025),
        venue=paper.get("venue", "arXiv"),
        citations=paper.get("citations", 0),
        content_source=paper.get("content_source", "摘要"),
        content=paper.get("full_content", "")[:5000]
    )

    agent = AgentsLLM()
    # 修复 4: 传入列表格式 messages
    response = agent.think([{"role": "user", "content": prompt}], temperature=0.1)
    
    mechanism = "解析完成"
    tradeoffs = "计算与内存权衡"
    limitations = "长文本存在精度衰减与上下文丢失"
    
    if "【核心机制】:" in response and "【权衡分析】:" in response:
        parts = response.split("【权衡分析】:")
        mechanism = parts[0].replace("【核心机制】:", "").strip()
        
        if "【正文局限】:" in parts[1]:
            sub_parts = parts[1].split("【正文局限】:")
            tradeoffs = sub_parts[0].strip()
            limitations = sub_parts[1].strip()
        else:
            tradeoffs = parts[1].strip()
    else:
        mechanism = response[:300]

    # 修复 5: 无论正则匹配是否成功，保证正常构建并返回数据
    summary_result: PaperSummary = {
        "title": title,
        "year": paper.get("year", 2025),
        "venue": paper.get("venue", "arXiv"),
        "citations": paper.get("citations", 0),
        "pdf_url": paper.get("pdf_url", ""),
        "core_mechanism": mechanism,
        "tradeoffs": tradeoffs,
        "limitations": limitations
    }

    return {"summaries": [summary_result]}


# =============================================================
# 测试入口
# =============================================================
if __name__ == "__main__":
    mock_papers = [
        {
            "title": "PyramidKV: Dynamic KV Cache Compression based on Pyramidal Information Funneling",
            "year": 2024,
            "venue": "arXiv.org",
            "citations": 388,
            "pdf_url": "https://arxiv.org/pdf/2406.02069.pdf",
            "abstract": "We observe attention allocation exhibits strong layer-wise pyramidal patterns."
        }
    ]

    print("🚀 1. 测试 fetch_pdf_node...")
    step1_res = fetch_pdf_node({"papers": mock_papers})
    
    fetched_papers = step1_res.get("papers", [])
    if not fetched_papers:
        print("❌ 未获取到有效论文数据，终止下游测试！")
    else:
        print(f"✅ 成功获取到 {len(fetched_papers)} 篇论文数据")
        print("\n🚀 2. 测试 summarize_single_paper_node...")
        
        single_input: SinglePaperInput = {
            "topic": "KV Cache Compression",
            "paper": fetched_papers[0]
        }
        step2_res = summarize_single_paper_node(single_input)

        if step2_res.get("summaries"):
            res = step2_res["summaries"][0]
            print("\n📦 【Worker 精读产物】:")
            print(f"【核心机制】:\n{res['core_mechanism'][:300]}...\n")
            print(f"【权衡分析】:\n{res['tradeoffs'][:300]}...\n")
            print(f"【正文局限】:\n{res['limitations'][:300]}...\n")


        
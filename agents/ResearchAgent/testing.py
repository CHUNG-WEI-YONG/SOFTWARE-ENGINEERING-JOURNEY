from SearchNode import search_semantic,search_arxiv,search_node,GraphState
import os
from Graph import summarize_node

def test_api():
    print("🔍 1. 测试 arXiv API 检索...")
    arxiv_res = search_arxiv(query="KV Cache Compression", max_result=2)
    print(f"arXiv 返回论文数量: {len(arxiv_res)}")
    if arxiv_res:
        print(f"  - 示例标题: {arxiv_res[0]['title']}")
        print(f"  - 示例年份: {arxiv_res[0]['year']}")

    print("\n🔍 2. 测试 Semantic Scholar API 检索...")
    semantic_res = search_semantic(query="KV Cache Compression", max_result=2)
    print(f"Semantic Scholar 返回论文数量: {len(semantic_res)}")
    if semantic_res and isinstance(semantic_res[0], dict):
        print(f"  - 示例标题: {semantic_res[0]['title']}")
        print(f"  - 示例引用量: {semantic_res[0]['citations']}")
        print(f"  - 示例会议: {semantic_res[0]['venue']}")

def test_search_node():
    print("\n" + "="*50)
    print("🚀 开始测试 search_node（ReAct 智能 Agent 检索）")
    print("="*50)

    # 模拟 LangGraph 输入的 State
    test_state: GraphState = {
        "topic": "KV Cache Compression in Large Language Models",
        "papers": [],
        "summary": "",
        "proposed_idea": "",
        "pass_evaluate": False,
        "retry_count": 0
    }

    # 运行 search_node
    result = search_node(test_state)

    papers = result.get("papers", [])
    print(f"\n✨ [节点执行结果] 成功获取并去重聚合了 {len(papers)} 篇论文：")
    for idx, p in enumerate(papers, 1):
        venue = p.get("venue") or "arXiv"
        print(f"{idx}. 《{p.get('title')}》 ({p.get('year')})")
        print(f"   - 来源/会议: {venue} | 引用量: {p.get('citations', 0)}")
        print(f"   - PDF: {p.get('pdf_url')}")

def test_summarize_node():
    # 模拟前面检索出来的真实数据
    mock_state: GraphState = {
        "topic": "KV Cache Compression in Large Language Models",
        "papers": [
            {
                "title": "PyramidKV: Dynamic KV Cache Compression based on Pyramidal Information Funneling",
                "year": 2024,
                "venue": "arXiv",
                "citations": 388,
                "abstract": "Recent works have shown that attention allocation exhibits strong layer-wise pyramidal patterns. We propose PyramidKV, dynamically allocating varying cache budgets across layers to compress KV cache without performance loss."
            },
            {
                "title": "KVzip: Query-Agnostic KV Cache Compression with Context Reconstruction",
                "year": 2025,
                "venue": "NeurIPS",
                "citations": 72,
                "abstract": "We present KVzip, a query-agnostic compression framework that reconstructs contextual representations using low-rank approximations, achieving 4x compression with minimal token distortion."
            }
        ],
        "summary": "",
        "proposed_idea": "",
        "pass_evaluate": False,
        "retry_count": 0
    }
    
    result = summarize_node(mock_state)
    print("\n--- 提取出的总结与缺陷 ---")
    print(result["summary"][:600] + "...\n")

if __name__ == "__main__":
    test_summarize_node()

if __name__ == "__main__":
    api_key = os.getenv("SEMANTIC_API_KEY")
    url = os.getenv("SEMANTIC_URL")

    print(f"API Key 是否读取到: {bool(api_key)}")
    if api_key:
        print(f"API Key 前4位与长度: {api_key[:4]}*** (总长: {len(api_key)})")
        print(f"请求的目标 URL: {url}")
        test_summarize_node()
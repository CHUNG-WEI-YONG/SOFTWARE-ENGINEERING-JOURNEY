from AiClient import AgentsLLM
import os 
import re

def export_node(state:dict)->dict:
    topic=state.get("topic","research_topic")
    safe_name = re.sub(r'[\\/*?:"<>| ]', "_", topic)
    filename = f"{safe_name}_research_proposal.md"

    print(f"\n" + "=" * 60)
    print(f"💾 [Export Node] 正在生成完整学术交付报告: {filename}")
    print(f"=" * 60)

    md = f"# 学术调研报告与顶会选题方案: {topic}\n\n"
    status_str = "✅ PASS (通过对抗查重与可行性验证)" if state.get("pass_evaluate") else "⚠️ 最大重试后自动归档"
    md += f"- **评审状态**: {status_str}\n"
    md += f"- **迭代轮次**: {state.get('retry_count', 1)}\n\n"
    md += "---\n\n"

    # 2. 核心文献精读对比矩阵
    md += "## 一、 核心文献精读对比矩阵 (Literature Matrix)\n\n"
    md += "| 论文标题 | 年份/会议 | 引用量 | 核心机制 | 权衡代价 | 正文披露局限性 |\n"
    md += "| :--- | :--- | :--- | :--- | :--- | :--- |\n"
    for s in state.get("summaries", []):
        t = s.get("title", "").replace("|", "-")
        y = s.get("year", 2025)
        v = s.get("venue", "arXiv")
        c = s.get("citations", 0)
        mech = s.get("core_mechanism", "")[:100].replace("\n", " ") + "..."
        trade = s.get("tradeoffs", "")[:60].replace("\n", " ") + "..."
        lim = s.get("limitations", "")[:100].replace("\n", " ") + "..."
        md += f"| **{t}** | {y} ({v}) | {c} | {mech} | {trade} | {lim} |\n"

    # 3. 拟定选题与提案（包含 PyTorch 算子代码与查重意见）
    md += "\n---\n\n## 二、 顶会创新方案与实验规划 (Research Proposal)\n\n"
    md += state.get("proposed_idea", "无提案内容")
    md += "\n\n---\n\n"

    md += "## 三、 参考文献 BibTeX (References)\n\n```bibtex\n"
    for idx, p in enumerate(state.get("papers", []), 1):
        clean_key = re.sub(r'\W+', '', p.get("title", f"paper{idx}")[:12]) + str(p.get("year", 2025))
        md += f"@article{{{clean_key},\n"
        md += f"  title={{{p.get('title')}}},\n"
        md += f"  year={{{p.get('year', 2025)}}},\n"
        md += f"  journal={{{p.get('venue') or 'arXiv preprint'}}},\n"
        md += f"  url={{{p.get('pdf_url', '')}}}\n"
        md += "}\n\n"
    md += "```\n"

    # 写入磁盘
    with open(filename, "w", encoding="utf-8") as f:
        f.write(md)

    print(f"✨ 报告生成完毕！文件保存在: {os.path.abspath(filename)}")
    return {}


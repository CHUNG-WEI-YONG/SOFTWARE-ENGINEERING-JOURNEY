import os
import re
from typing import Dict
from AiClient import AgentsLLM


def export_node(state: Dict) -> Dict:
    topic = state.get("topic", "research_topic")
    safe_name = re.sub(r'[\\/*?:"<>| ]', "_", topic)
    filename = f"{safe_name}_research_proposal.md"

    print(f"\n" + "=" * 60)
    print(f"💾 [Export Node] Generating academic proposal report: {filename}")
    print(f"=" * 60)

    md = f"# Academic Research Report & Proposal: {topic}\n\n"
    status_str = "✅ PASS (Quality Check & Feasibility Verified)" if state.get("pass_evaluate") else "⚠️ Archived after Max Retries"
    md += f"- **Review Status**: {status_str}\n"
    md += f"- **Iteration Round**: {state.get('retry_count', 1)}\n\n"
    md += "---\n\n"

    # 1. Literature Matrix
    md += "## 1. Literature Matrix\n\n"
    md += "| Paper Title | Year / Venue | Citations | Core Mechanism | Trade-offs | Disclosed Limitations |\n"
    md += "| :--- | :--- | :--- | :--- | :--- | :--- |\n"
    for s in state.get("summaries", []):
        t = s.get("title", "").replace("|", "-")
        y = s.get("year", 2026)
        v = s.get("venue", "arXiv")
        c = s.get("citations", 0)
        mech = s.get("core_mechanism", "")[:100].replace("\n", " ") + "..."
        trade = s.get("tradeoffs", "")[:60].replace("\n", " ") + "..."
        lim = s.get("limitations", "")[:100].replace("\n", " ") + "..."
        md += f"| **{t}** | {y} ({v}) | {c} | {mech} | {trade} | {lim} |\n"

    # 2. Research Proposal & Innovation Scheme
    md += "\n---\n\n## 2. Research Proposal & Experimental Setup\n\n"
    md += state.get("proposed_idea", "No proposal content generated.")
    md += "\n\n---\n\n"

    # 3. BibTeX References
    md += "## 3. BibTeX References\n\n```bibtex\n"
    for idx, p in enumerate(state.get("papers", []), 1):
        clean_key = re.sub(r'\W+', '', p.get("title", f"paper{idx}")[:12]) + str(p.get("year", 2026))
        md += f"@article{{{clean_key},\n"
        md += f"  title={{{p.get('title')}}},\n"
        md += f"  year={{{p.get('year', 2026)}}},\n"
        md += f"  journal={{{p.get('venue') or 'arXiv preprint'}}},\n"
        md += f"  url={{{p.get('pdf_url', '')}}}\n"
        md += "}\n\n"
    md += "```\n"

    # Write report to disk
    with open(filename, "w", encoding="utf-8") as f:
        f.write(md)

    print(f"✨ Report successfully generated! Saved to: {os.path.abspath(filename)}")
    return {}


import os
import operator
from typing import TypedDict, List, Dict, Optional, Annotated
from dotenv import load_dotenv

from langgraph.graph import StateGraph, START, END
from langgraph.types import Send
from langgraph.checkpoint.memory import MemorySaver

# Internal project imports
from AiClient import AgentsLLM
from SearchNode import search_node
from fetchpdf import fetch_pdf_node, summarize_single_paper_node, PaperSummary
from EvaluatorNode import evaluate_node
from ExportNode import export_node

load_dotenv()


# =============================================================
# 1. State Definition
# =============================================================
class GraphState(TypedDict):
    topic: str
    papers: List[Dict]
    # operator.add aggregates worker results dynamically into a single list
    summaries: Annotated[List[PaperSummary], operator.add]
    summary: str
    proposed_idea: str
    pass_evaluate: bool
    retry_count: int


# =============================================================
# 2. Dynamic Worker Distribution (Map) & Conditional Routing
# =============================================================
def distribute_to_worker(state: GraphState):
    """
    Map Phase: Dynamically fans out single-paper extraction tasks to worker nodes.
    """
    papers = state.get("papers", [])
    topic = state.get("topic", "")
    print(f"\n⚡ [Map Fan-Out] Distributing {len(papers)} papers across parallel worker agents...")
    return [
        Send("summary", {"topic": topic, "paper": p})
        for p in papers
    ]


def route_after_evaluator(state: dict) -> str:
    """
    Conditional Routing: Checks evaluation criteria to decide whether to export or loop back.
    """
    if state.get("pass_evaluate", False) or state.get("retry_count", 0) >= 2:
        return "export_node"
    
    print("\n🔁 [Loop Triggered] Proposal failed quality check. Looping back to search node...")
    return "search_node"


# =============================================================
# 3. LangGraph Workflow Definition & Wiring
# =============================================================
workflow = StateGraph(GraphState)

# Register Pipeline Nodes
workflow.add_node("search", search_node)
workflow.add_node("fetch", fetch_pdf_node)
workflow.add_node("summary", summarize_single_paper_node)
workflow.add_node("evaluate", evaluate_node)
workflow.add_node("export", export_node)

# Linear Edges: Start -> Search -> Fetch PDF
workflow.add_edge(START, "search")
workflow.add_edge("search", "fetch")

# Dynamic Fan-Out Edge: Distribute fetched papers to summary workers
workflow.add_conditional_edges("fetch", distribute_to_worker, ["summary"])

# Reduce Aggregation: Workers merge results and flow into evaluator
workflow.add_edge("summary", "evaluate")

# Conditional Routing: Evaluate -> (Export | Search)
workflow.add_conditional_edges(
    "evaluate",
    route_after_evaluator,
    {
        "export_node": "export",
        "search_node": "search"
    }
)

# Terminal Edge
workflow.add_edge("export", END)

# Compile Graph with Memory Checkpointing
app = workflow.compile(checkpointer=MemorySaver())


# =============================================================
# 4. Pipeline Execution Entry Point
# =============================================================
if __name__ == "__main__":
    question=input("Enter the research topic you want to study: ").strip()
    initial_input: GraphState = {
        "topic": question,
        "papers": [],
        "summaries": [],
        "summary": "",
        "proposed_idea": "",
        "pass_evaluate": False,
        "retry_count": 0
    }

    config = {"configurable": {"thread_id": "research_run_001"}}

    print("\n🚀 Starting automated academic research pipeline...")
    for step in app.stream(initial_input, config=config):
        pass
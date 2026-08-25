from typing import TypedDict,List,Dict ,Optional,Annotated
from langgraph.graph import StateGraph, START, END
import operator
from langgraph.checkpoint.memory import MemorySaver
import arxiv
import requests
from dotenv import load_dotenv
import os
from langgraph.types import Send
from SearchNode import search_node
from AiClient import AgentsLLM
from ExportNode import export_node
from fetchpdf import fetch_pdf_node,summarize_single_paper_node,PaperSummary
from EvaluatorNode import evaluate_node


load_dotenv()

def route_after_evaluator(state: dict) -> str:
    if state.get("pass_evaluate", False) or state.get("retry_count", 0) >= 2:
        return "export_node"
    
    print("\n🔁 [触发回流] 提案未通过质检，回流重新执行检索与构思...")
    return "search_node"


class GraphState(TypedDict):
    topic:str
    papers:List[dict]
    summaries: Annotated[List[PaperSummary], operator.add]
    summary:str
    proposed_idea:str
    pass_evaluate:bool
    retry_count:int


def distribute_to_worker(state:GraphState):
    papers=state.get("papers",[])
    topic=state.get("topic","")
    return [
        Send("summary",{"topic":topic,"paper":p}) for p in papers
    ]

# -------------------------------------------------------------
# 3. Link & Routing: 条件路由逻辑
# -------------------------------------------------------------


workflow=StateGraph(GraphState)

workflow.add_node("search",search_node)
workflow.add_node("summary",summarize_single_paper_node)
workflow.add_node("fetch",fetch_pdf_node)
workflow.add_node("evaluate",evaluate_node)
workflow.add_node("export",export_node)

workflow.add_edge(START,"search")
workflow.add_edge("search","fetch")
workflow.add_conditional_edges("fetch",distribute_to_worker,["summary"])
workflow.add_edge("summary","evaluate")

workflow.add_conditional_edges("evaluate",
            route_after_evaluator,
            {
                "export_node":"export",
                "search_node":"search"
            })
workflow.add_edge("export",END)

app=workflow.compile(checkpointer=MemorySaver())

if __name__ == "__main__":
    initial_input: GraphState = {
        "topic": "KV Cache Compression in Large Language Models",
        "papers": [],
        "summaries": [],
        "summary": "",
        "proposed_idea": "",
        "pass_evaluate": False,
        "retry_count": 0
    }

    config = {"configurable": {"thread_id": "research_run_001"}}

    print("\n🚀 启动学术研究自动化流水线...")
    # 流式触发执行图
    for step in app.stream(initial_input, config=config):
        pass



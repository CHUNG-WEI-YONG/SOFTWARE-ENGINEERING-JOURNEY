import os
import re
import math
from typing import TypedDict, List, Dict, Optional, Callable, Any
from dotenv import load_dotenv
import arxiv
import requests

from AiClient import AgentsLLM, ToolExecutor, ReAct_Agent

load_dotenv()


class GraphState(TypedDict):
    topic: str
    papers: List[Dict]
    summary: str
    proposed_idea: str
    pass_evaluate: bool
    retry_count: int


SEARCH_REACT_PROMPT = """You are an academic search agent. Given a user topic, formulate 2-3 precise academic search terms (e.g., 'Code LLM API documentation', 'Retrieval Augmented Code Generation') and call retrieval tools.

Available Tools:
{tools}

Format:
Thought: analyze keyword strategy
Action: tool_name[clean search keyword without parameter names or quotes]
Observation: tool output
... (repeat if needed)
Thought: I have collected enough papers.
Action: Finish[done]

Task:
Question: {question}

History:
{history}
"""


def search_arxiv(query: str, max_result: int = 5) -> List[Dict]:
    clean_query = re.sub(r'^(query\s*=\s*|search_query\s*=\s*)', '', query, flags=re.IGNORECASE)
    clean_query = clean_query.split(",")[0].strip().strip("'").strip('"')
    clean_query = " ".join(clean_query.split()[:6])

    if not clean_query:
        return []

    papers = []
    try:
        client = arxiv.Client(page_size=int(max_result), delay_seconds=2.0, num_retries=2)
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
        print(f"⚠️ [arXiv API] Handled exception: {e}")
        return []

    return papers


def search_semantic(query: str, max_result: int = 5) -> List[Dict]:
    clean_query = re.sub(r'^(query\s*=\s*|search_query\s*=\s*)', '', query, flags=re.IGNORECASE)
    clean_query = clean_query.split(",")[0].strip().strip("'").strip('"')
    clean_query = " ".join(clean_query.split()[:6])

    if not clean_query:
        return []

    api_key = os.getenv("SEMANTIC_API_KEY")
    url = "https://api.semanticscholar.org/graph/v1/paper/search"
    headers = {"User-Agent": "Mozilla/5.0 ResearchAgent/1.0"}
    if api_key:
        headers["x-api-key"] = api_key.strip()

    params = {
        "query": clean_query,
        "limit": int(max_result),
        "fields": "title,authors,year,citationCount,venue,tldr,abstract,openAccessPdf",
    }

    try:
        response = requests.get(url=url, headers=headers, params=params, timeout=10)
        if response.status_code == 200:
            data = response.json().get("data", [])
            papers = []
            for item in data:
                pdf_info = item.get("openAccessPdf") or {}
                if item.get("title"):
                    papers.append({
                        "title": item.get("title").strip(),
                        "year": item.get("year") or 2024,
                        "citations": item.get("citationCount", 0),
                        "venue": item.get("venue", "") or "Semantic Scholar",
                        "tldr": (item.get("tldr") or {}).get("text", ""),
                        "abstract": (item.get("abstract") or "").replace("\n", " ").strip(),
                        "pdf_url": pdf_info.get("url") or ""
                    })
            return papers
    except Exception as e:
        print(f"⚠️ [Semantic Scholar API] Handled exception: {e}")

    return []


class SearchAgent(ReAct_Agent):
    def __init__(self, client: AgentsLLM, tools: ToolExecutor, prompt: str, max_time: int = 4):
        super().__init__(client, tools, prompt, max_time)
        self.collected_papers: List[Dict] = []
        self.max_steps = max_time

    def _parse_output(self, text: str):
        thought_match = re.search(r"Thought:\s*(.*?)(?=\nAction:|$)", text, re.DOTALL | re.IGNORECASE)
        action_match = re.search(r"Action:\s*(.*?)$", text, re.DOTALL | re.IGNORECASE)
        thought = thought_match.group(1).strip() if thought_match else ""
        action = action_match.group(1).strip() if action_match else ""
        return thought, action

    def _parse_action(self, action_text: str):
        # Cleans backticks and extra wrappers
        clean_text = action_text.strip().strip("`").strip()
        match = re.search(r"(\w+)\[(.*)\]", clean_text, re.DOTALL)
        if match:
            return match.group(1).strip(), match.group(2).strip()
        return None, None

    def generate(self, question: str) -> List[Dict]:
        self.history = []
        self.collected_papers = []
        curr_step = 0

        while curr_step < self.max_steps:
            curr_step += 1
            tools_str = self.tools.getInfo()
            history_str = "\n".join(self.history) if self.history else "None"
            prompt = self.prompt.format(
                tools=tools_str,
                question=question,
                history=history_str
            )

            response = self.client.think([{"role": "user", "content": prompt}], stream=False)
            if not response:
                break

            thought, action = self._parse_output(response)
            if not action or action.lower().startswith("finish"):
                break

            func_name, argument = self._parse_action(action)
            if not func_name:
                break

            tool_func = self.tools.getTools(func_name)
            if not tool_func:
                continue

            # Strip any residual parameter assignments inside brackets
            clean_arg = re.sub(r'^(query\s*=\s*|search_query\s*=\s*)', '', argument, flags=re.IGNORECASE)
            clean_arg = clean_arg.split(",")[0].strip().strip("'").strip('"')

            print(f"🎬 [Agent Invoking Tool] {func_name} -> Query: '{clean_arg}'")
            papers_found = tool_func(query=clean_arg, max_result=5)

            new_count = 0
            if isinstance(papers_found, list) and papers_found:
                for p in papers_found:
                    if isinstance(p, dict) and p.get("title"):
                        if not any(existing['title'].lower() == p['title'].lower() for existing in self.collected_papers):
                            self.collected_papers.append(p)
                            new_count += 1
                observation = f"Fetched {len(papers_found)} papers ({new_count} new)."
            else:
                observation = "No papers found with this query."

            self.history.append(f"Thought: {thought}")
            self.history.append(f"Action: {func_name}[{clean_arg}]")
            self.history.append(f"Observation: {observation}")

        return self.collected_papers


def search_node(state: dict) -> dict:
    topic = state.get("topic", "")
    print(f"\n🚀 Executing literature search node for topic: {topic}")

    agent = AgentsLLM()
    tools = ToolExecutor()

    tools.register_tool("search_arxiv", search_arxiv, "Search arXiv preprints. Usage: search_arxiv[keyword]")
    tools.register_tool("search_semantic", search_semantic, "Search Semantic Scholar papers. Usage: search_semantic[keyword]")

    # 1. Pre-extract concise academic keywords from conversational queries
    kw_prompt = [
        {"role": "system", "content": "Extract 2-3 precise academic search keywords (comma-separated) for an arXiv/Semantic Scholar query based on the user's research topic. Return ONLY the comma-separated keywords."},
        {"role": "user", "content": f"Topic: {topic}"}
    ]
    extracted_keywords = agent.think(kw_prompt).strip().replace("\n", " ")
    print(f"🔍 Extracted Academic Query Keywords: {extracted_keywords}")

    # 2. Run ReAct Search Agent
    search_agent = SearchAgent(client=agent, tools=tools, prompt=SEARCH_REACT_PROMPT, max_time=4)
    papers = search_agent.generate(f"Topic: {topic}. Target Keywords: {extracted_keywords}")

    # 3. Dynamic Fallback: Search using the primary extracted keyword if agent returned empty
    if not papers:
        print("⚠️ ReAct loop returned 0 papers. Running direct fallback query with primary keyword...")
        primary_kw = extracted_keywords.split(",")[0].strip()
        papers = search_arxiv(primary_kw, max_result=4)
        if not papers:
            papers = search_semantic(primary_kw, max_result=4)

    print(f"✅ Search complete. Aggregated {len(papers)} papers.")
    return {"papers": papers[:5]}




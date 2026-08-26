# 🚀 ResearchAgent: Autonomous Academic Literature Survey & Proposal System

<p align="center">
  <img src="https://img.shields.io/badge/Python-3.10%2B-blue.svg" alt="Python Version">
  <img src="https://img.shields.io/badge/LangGraph-1.2%2B-green.svg" alt="LangGraph">
  <img src="https://img.shields.io/badge/ArXiv-API-red.svg" alt="ArXiv">
  <img src="https://img.shields.io/badge/License-MIT-purple.svg" alt="License">
</p>

**ResearchAgent** is an end-to-end autonomous research workflow powered by **LangGraph** state machines and **ReAct Agents**. The system coordinates literature retrieval, full-text parsing, parallel worker analysis (Map-Reduce), adversarial peer-review evaluation, and markdown proposal generation with standard BibTeX citations.

---

## 🌟 Key Features

* **Intelligent Academic Search**: Utilizes ReAct agents connected to arXiv and Semantic Scholar APIs with query sanitization, deduplication, and HTTP 429 rate-limit handling.
* **Dynamic Map-Reduce Fan-Out**: Employs LangGraph's `Send` API to parallelize paper analysis across multiple independent worker nodes, preventing context-window overflow.
* **Structured Literature Matrix**: Extracts mechanisms, computational trade-offs, citation metrics, and explicit limitations into a comparative matrix.
* **Adversarial Peer Review**: Simulates top-tier conference peer review to evaluate novelty, compute feasibility, and baseline rigor with automated self-correction loops.
* **Automated Asset Export**: Compiles findings into an Overleaf-ready proposal with structured tables, methodology proposals, and verified BibTeX citations.

---

## 🛠️ Architecture & Workflow

```text
       [ START ]
           │
           ▼
    ┌─────────────┐
    │ search_node │  <── (ReAct Retrieval & Deduplication)
    └──────┬──────┘
           │
           ▼
    ┌─────────────┐
    │  fetch_pdf  │  <── (PDF Fetching & Text Extraction)
    └──────┬──────┘
           │
   [ Send Fan-Out (Map) ]
     ┌─────┴─────────────────┐
     ▼                       ▼
┌───────────────┐     ┌───────────────┐
│ worker_node_1 │ ... │ worker_node_n │  <── (Parallel Paper Analysis)
└──────┬────────┘     └───────┬───────┘
     └─────────────┬─────────┘
           │ [ operator.add Aggregation (Reduce) ]
           ▼
    ┌─────────────┐
    │  evaluator  │  <── (Peer Review & Novelty Verification)
    └──────┬──────┘
      /          \
  [Pass]       [Fail & retry < 2]
    /              \
   ▼                ▼
┌─────────────┐   (Loop back to search_node)
│ export_node │
└──────┬──────┘
       │
       ▼
    [ END ]
```



## 📁 Project Structure
Plaintext
agents/
├── AiClient.py          # LLM base client, ToolExecutor, and ReAct agent runtime
├── SearchNode.py        # Academic search tools (arXiv / Semantic Scholar) & retrieval logic
├── fetchpdf.py          # PDF document downloading and single-paper analysis workers
├── EvaluatorNode.py     # Evaluation node, quality control, and proposal drafting
├── ExportNode.py        # Markdown report generation and BibTeX compiler
├── Graph.py             # LangGraph state machine topology and execution entry point
├── .env                 # Environment variables and API credentials
└── requirements.txt     # Python dependency specifications


## 🚀 Quick Start
1. Clone & Set Up Environment
PowerShell
# Clone the repository
``` text
git clone [https://github.com/your-username/ResearchAgent.git](https://github.com/your-username/ResearchAgent.git)
cd ResearchAgent 
```


# Create virtual environment
```text
python -m venv .venv
```

# Activate virtual environment (Windows PowerShell)
```text
.venv\Scripts\Activate.ps1
```

# Activate virtual environment (Linux / macOS)
# source .venv/bin/activate

# Install dependencies
```text
pip install -r requirements.txt
2. Configure Environment Variables
Create a .env file in the project root directory:

Code snippet
API_KEY="your_api_key_here"
BASE_URL="[https://dashscope.aliyuncs.com/compatible-mode/v1](https://dashscope.aliyuncs.com/compatible-mode/v1)"
MODEL_ID="qwen-max"
LLM_TIMEOUT=60

# Optional: Semantic Scholar API Key for higher rate limits
SEMANTIC_API_KEY="your_semantic_scholar_api_key"

3. Run the Research Pipeline
PowerShell
python agents/Graph.py
Upon execution, the full research report and proposal will be exported to your working directory as a formatted Markdown file (e.g., KV_Cache_Compression_in_Large_Language_Models_research_proposal.md).

```

## 📋 Dependencies (requirements.txt)
```text 
Plaintext
langgraph>=1.2.0
langchain-core>=1.4.0
arxiv>=2.1.0
requests>=2.31.0
python-dotenv>=1.0.0
pymupdf>=1.24.0
openai>=1.40.0
pydantic>=2.7.0
```
## Output Report Format

The generated proposal document includes:

Literature Matrix: Year, conference/venue, citation count, core mechanism, computational trade-offs, and explicit limitations.

Deep Research Gaps: Multi-paper synthesis of underlying algorithmic and architectural bottlenecks.

Research Proposal: Proposed title, mathematical formulation, PyTorch pseudo-operator design, baseline models, and evaluation benchmarks.

BibTeX References: Clean, formatted BibTeX entries ready for Overleaf and LaTeX integration.
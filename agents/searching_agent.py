import os
from AiClient import AgentsLLM,ToolExecutor,ReAct_Agent
from dotenv import load_dotenv
from openai import OpenAI
from serpapi import SerpApiClient

load_dotenv()

# ReAct 提示词模板


def search(query:str):
    print(f"Searching for {query}")
    try:
        search_api=os.getenv("SERPAPI_KEY")
        if not search_api:
            return "Error: no api included"
        params = {
            "engine": "google",
            "q": query,
            "api_key": search_api,
            "gl": "cn",  # 国家代码
            "hl": "zh-cn", # 语言代码
        }
        search_user=SerpApiClient(params)
        results=search_user.get_dict()
        if "answer_box_list" in results:
            return "\n".join(results["answer_box_list"])
        if "answer_box" in results and "answer" in results["answer_box"]:
            return results["answer_box"]["answer"]
        if "knowledge_graph" in results and "description" in results["knowledge_graph"]:
            return results["knowledge_graph"]["description"]
        if "organic_results" in results and results["organic_results"]:
            # 如果没有直接答案，则返回前三个有机结果的摘要
            snippets = [
                f"[{i+1}] {res.get('title', '')}\n{res.get('snippet', '')}"
                for i, res in enumerate(results["organic_results"][:3])
            ]
            return "\n\n".join(snippets)
        return f"Sorry , no result for {query}"

    except Exception as e:
        return f"搜索时发生错误: {e}"

    
        

user=AgentsLLM()
tool=ToolExecutor()
search_description = "一个网页搜索引擎。当你需要回答关于时事、事实以及在你的知识库中找不到的信息时，应使用此工具。"
tool.InsertTool("Search",search,search_description)

agent=ReAct_Agent(user,tool)
user_question=input("What is your question about the world?\n")
agent.generate(user_question)


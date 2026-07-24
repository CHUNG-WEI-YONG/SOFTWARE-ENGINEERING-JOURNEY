import requests
import os
from tavily import TavilyClient
from openai import OpenAI
import re



os.environ['TAVILY_API_KEY'] = "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"


# ⚠️ 替换成你刚注册的 DeepSeek Key，千万别再发出来啦！


AGENT_SYSTEM_PROMPT="""你是一个智能旅行助手。你的任务是分析用户的请求，并使用可用工具一步步地解决问题。

# 可用工具:
- `get_weather(city: str)`: 查询指定城市的实时天气。
- `get_attraction(city: str, weather: str)`: 根据城市和天气搜索推荐的旅游景点。

# 输出格式要求:
你的每次回复必须严格遵循以下格式，包含一对Thought和Action：

Thought: [你的思考过程和下一步计划]
Action: [你要执行的具体行动]

Action的格式必须是以下之一：
1. 调用工具：function_name(arg_name="arg_value")
2. 结束任务：Finish[最终答案]

# 重要提示:
- 每次只输出一对Thought-Action
- Action必须在同一行，不要换行
- 当收集到足够信息可以回答用户问题时，必须使用 Action: Finish[最终答案] 格式结束

请开始吧！
"""

def get_weather(city: str) -> str:
    # 替换空格为 +，确保多词城市名（如 New York）正常解析
    safe_city = city.replace(" ", "+")
    url = f"https://wttr.in/{safe_city}?format=j1"
    
    # 增加伪装 Header 和超时限制（5秒）
    headers = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"}
    
    try:
        response = requests.get(url, headers=headers, timeout=5)
        response.raise_for_status()
        data = response.json()
        
        current_condition = data["current_condition"][0]
        weather_desc = current_condition['weatherDesc'][0]['value']
        temp_c = current_condition['temp_C']
        return f"{city}当前天气: {weather_desc}，气温 {temp_c} 摄氏度"
        
    except requests.exceptions.Timeout:
        return f"错误: 查询 {city} 天气超时，请重试。"
    except requests.exceptions.RequestException as e:
        return f"错误: 查询天气时遇到网络问题 - {e}"
    except (KeyError, IndexError) as e:
        return f"错误: 解析天气数据失败，可能是城市名称 '{city}' 无效。"
    
def get_attraction(city: str, weather: str) -> str:
    """
    根据城市和天气，使用Tavily Search API搜索并返回优化后的景点推荐。
    """
    # 1. 从环境变量中读取API密钥
    api_key = os.environ.get("TAVILY_API_KEY")
    if not api_key:
        return "错误:未配置TAVILY_API_KEY环境变量。"

    # 2. 初始化Tavily客户端
    tavily = TavilyClient(api_key=api_key)
    
    # 3. 构造一个精确的查询
    query = f"'{city}' 在'{weather}'天气下最值得去的旅游景点推荐及理由"
    
    try:
        # 4. 调用API，include_answer=True会返回一个综合性的回答
        response = tavily.search(query=query, search_depth="basic", include_answer=True)
        
        # 5. Tavily返回的结果已经非常干净，可以直接使用
        # response['answer'] 是一个基于所有搜索结果的总结性回答
        if response.get("answer"):
            return response["answer"]
        
        # 如果没有综合性回答，则格式化原始结果
        formatted_results = []
        for result in response.get("results", []):
            formatted_results.append(f"- {result['title']}: {result['content']}")
        
        if not formatted_results:
             return "抱歉，没有找到相关的旅游景点推荐。"

        return "根据搜索，为您找到以下信息:\n" + "\n".join(formatted_results)

    except Exception as e:
        return f"错误:执行Tavily搜索时出现问题 - {e}"
    
available_tools={
    "get_weather":get_weather,
    "get_attraction":get_attraction
}


class OpenAIClient:
    def __init__(self,model:str,api:str,base_url:str):
        self.model=model
        self.client=OpenAI(api_key=api,base_url=base_url)

    def generate(self,prompt,system_prompt):
         print("正在调用大语言模型...")
         try:
             message=[{"role":"system","content":system_prompt},
                      {"role":"user","content":prompt}
                    ]
             response=self.client.chat.completions.create(
                 model=self.model,
                 messages=message,
                 stream=False,
             )
             answer=response.choices[0].message.content
             print("correct model response")
             return answer
         except Exception as e:
            print(f"调用LLM API时发生错误: {e}")
            return "错误:调用语言模型服务时出错。"

llm=OpenAIClient(MODEL_ID,API_KEY,BASE_URL)
destination=input("Enter a place:")
user_prompt = f"你好，请帮我查询一下今天{destination}的天气，然后根据天气推荐一个合适的旅游景点。"  
prompt_history = [f"用户请求: {user_prompt}"]

print(f"用户输入: {user_prompt}\n" + "="*40) 
for i in range(5):
    print(f"Round {i+1}")
    full_prompt="\n".join(prompt_history)

    llm_output=llm.generate(full_prompt,AGENT_SYSTEM_PROMPT)
    match = re.search(r'(Thought:.*?Action:.*?)(?=\n\s*(?:Thought:|Action:|Observation:)|\Z)', llm_output, re.DOTALL)
    if match:
        truncated = match.group(1).strip()
        if truncated != llm_output.strip():
            llm_output = truncated
            print("已截断多余的 Thought-Action 对")
    print(f"模型输出:\n{llm_output}\n")
    prompt_history.append(llm_output)
    
    action_match=re.search(r"Action: (.*)", llm_output, re.DOTALL)
    if not action_match:
        observation = "错误: 未能解析到 Action 字段。请确保你的回复严格遵循 'Thought: ... Action: ...' 的格式。"
        observation_str = f"Observation: {observation}"
        print(f"{observation_str}\n" + "="*40)
        prompt_history.append(observation_str)
        continue
    action_str=action_match.group(1).strip()

    if action_str.startswith("Finish"):
        final_answer = re.match(r"Finish\[(.*)\]", action_str).group(1)
        print(f"任务完成，最终答案: {final_answer}")
        break
    else:
        tool_name = re.search(r"(\w+)\(", action_str).group(1)
        args_str = re.search(r"\((.*)\)", action_str).group(1)
        kwargs = dict(re.findall(r'(\w+)="([^"]*)"', args_str))

        if tool_name in available_tools:
            observation=available_tools[tool_name](**kwargs)
        else:
            observation = f"错误:未定义的工具 '{tool_name}'"

    observation_str=f"Observation : {observation}"
    print(f"{observation_str}\n" + "="*40)
    prompt_history.append(observation_str)
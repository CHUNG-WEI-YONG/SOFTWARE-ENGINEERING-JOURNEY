from autogen_ext.models.openai import OpenAIChatCompletionClient
from dotenv import load_dotenv
import os
import asyncio
from autogen_core.models import ModelInfo

load_dotenv()

from autogen_ext.models.openai import OpenAIChatCompletionClient
from autogen_agentchat.agents import AssistantAgent, UserProxyAgent
from autogen_agentchat.teams import RoundRobinGroupChat
from autogen_agentchat.conditions import TextMentionTermination
from autogen_agentchat.ui import Console


def createAIClient():
    model_name = os.getenv("MODEL_ID1", "gpt-4o")
    return OpenAIChatCompletionClient(
        model=model_name,
        api_key=os.getenv("API_KEY1"),
        base_url=os.getenv("BASE_URL1", "https://api.openai.com/v1"),
        model_info=ModelInfo(
            vision=False,  # Set to True if your model supports image inputs
            function_calling=True,  # Set to True if your model supports tool calls
            json_output=True,  # Set to True if your model supports JSON mode
            family="unknown",  # Model family identifier
        ),
    )

def create_product_manager(model_client):
    system_message = """你是一位经验丰富的产品经理，专门负责软件产品的需求分析和项目规划。

    你的核心职责包括：
    1. **需求分析**：深入理解用户需求，识别核心功能和边界条件
    2. **技术规划**：基于需求制定清晰的技术实现路径
    3. **风险评估**：识别潜在的技术风险和用户体验问题
    4. **协调沟通**：与工程师和其他团队成员进行有效沟通

    当接到开发任务时，请按以下结构进行分析：
    1. 需求理解与分析
    2. 功能模块划分
    3. 技术选型建议
    4. 实现优先级排序
    5. 验收标准定义

    请简洁明了地回应，并在分析完成后说"请工程师开始实现"。"""

    return AssistantAgent(
        name="ProductManager",
        model_client=model_client,
        system_message=system_message
    )

def create_engineer(model_client):
    system_message = """你是一位资深的软件工程师，擅长 Python 开发和 Web 应用构建。

    你的技术专长包括：
    1. **Python 编程**：熟练掌握 Python 语法和最佳实践
    2. **Web 开发**：精通 Streamlit、Flask、Django 等框架
    3. **API 集成**：有丰富的第三方 API 集成经验
    4. **错误处理**：注重代码的健壮性和异常处理

    当收到开发任务时，请：
    1. 仔细分析技术需求
    2. 选择合适的技术方案
    3. 编写完整的代码实现
    4. 添加必要的注释和说明
    5. 考虑边界情况和异常处理

    请提供完整的可运行代码，并在完成后说"请代码审查员检查"。"""
    return AssistantAgent(
        name="Engineer",
        model_client=model_client,
        system_message=system_message
    )

def create_code_viewer(model_client):
    """创建代码审查员智能体"""
    system_message = """你是一位经验丰富的代码审查专家，专注于代码质量和最佳实践。

你的审查重点包括：
1. **代码质量**：检查代码的可读性、可维护性和性能
2. **安全性**：识别潜在的安全漏洞和风险点
3. **最佳实践**：确保代码遵循行业标准和最佳实践
4. **错误处理**：验证异常处理的完整性和合理性

审查流程：
1. 仔细阅读和理解代码逻辑
2. 检查代码规范和最佳实践
3. 识别潜在问题和改进点
4. 提供具体的修改建议
5. 评估代码的整体质量

请提供具体的审查意见，完成后说"代码审查完成，请用户代理测试"。"""
    return AssistantAgent(
        name="CodeReviewer",
        model_client=model_client,
        system_message=system_message
    )

def create_user_proxy():
    return UserProxyAgent(
        name="UserProxy",
        description="""用户代理，负责以下职责：
1. 代表用户提出开发需求
2. 执行最终的代码实现
3. 验证功能是否符合预期
4. 提供用户反馈和建议

完成测试后请回复 TERMINATE。""",
    )

user=createAIClient()
product_manager=create_product_manager(user)
engineer=create_engineer(user)
code_reviewer=create_code_viewer(user)
user_proxy=create_user_proxy()

team_chats=RoundRobinGroupChat(
    participants=[product_manager,engineer,code_reviewer,user_proxy],
    termination_condition=TextMentionTermination("TERMINATE"),
    max_turns=20,
)

async def run_software_Development_team():
    task = """我们需要开发一个针对 Google Careers IT 岗位抓取与求职建议应用，具体要求如下：

核心功能：
1. 实时获取 Google Careers 上最新发布的 IT / 软件工程相关岗位列表。
2. 提炼并明确列出目标岗位的具体 Requirements (任职要求) 与 Qualifications (加分项)。
3. 针对岗位需求，推荐适合写进简历中的项目经历（Project Recommendations）。
4. 针对该岗位提供具体的面试准备建议与学习路线（Interview Preparation Advice）。

技术与协作要求：
- 使用 Python 编写核心抓取与分析逻辑（可利用 API 或 BeautifulSoup/Playwright）。
- 提供简洁清晰的终端输出格式或 Web 界面结构。
- 包含适当的错误处理（如网络请求失败、网页结构变动防护）。
- 请团队按“需求分析 -> 代码实现与抓取 -> 简历与面试策略生成”流程协作完成。
"""

    result=await Console(team_chats.run_stream(task=task))
    return result

if __name__=="__main__":
    result=asyncio.run(run_software_Development_team())



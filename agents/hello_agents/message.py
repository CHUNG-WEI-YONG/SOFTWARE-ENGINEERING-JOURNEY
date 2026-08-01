from typing import Optional, Dict, Any, Literal
from datetime import datetime
from pydantic import BaseModel

MessageRole=Literal["user","system","assistant","tool"]

class Message(BaseModel):
    content:str
    role:MessageRole
    time_stamp:datetime=None
    meta_data:Optional[Dict[str,any]]=None

    def __init__(self, content: str, role: MessageRole, **kwargs):
        super().__init__(
            content=content,
            role=role,
            timestamp=kwargs.get('timestamp', datetime.now()),
            metadata=kwargs.get('metadata', {})
        )

    def to_dict(self):
        return {
            "role":self.role,
            "content":self.content
        }

    def __str__(self):
        return f"[{self.role}] {self.content}"
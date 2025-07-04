# schemas.py

from __future__ import annotations
from pydantic import BaseModel, Field, RootModel
from datetime import datetime
from typing import Dict, Any, Optional, List, Tuple

JsonDict = Dict[str, Any]

# -------------- api / queue ------------------
class EnqueueRequest(RootModel):
    root: Dict[str, Any]

class EnqueueResponse(BaseModel):
    task_id: str

class QueueItem(BaseModel):
    id: str
    priority: int

class QueueStateResponse(BaseModel):
    running: Dict[int, QueueItem]
    pending: List[Tuple[int, QueueItem]]

class HistoryItem(BaseModel):
    task: Dict[str, Any]
    status: Dict[str, Any]

class ProgressPayload(BaseModel):
    type: str
    data: Dict[str, Any]

class NodeListResponse(BaseModel):
    flag: str
    message: str
    result: List[Any]

class WorkFlowSaveResponse(BaseModel):
    flag: str
    message: str
    result: Dict[str, str]

class UploadResponse(BaseModel):
    filename: str
    saved_path: str
    size: int
    uploaded_at: datetime

class PreviewPayload(BaseModel):
    type: Literal["preview"]
    data: Dict[str, Any]

UploadResponse.model_rebuild()

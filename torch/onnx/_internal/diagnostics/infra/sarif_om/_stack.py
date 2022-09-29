# DO NOT EDIT! This file was generated by jschema_to_python version 0.0.1.dev29,
# with extension for dataclasses and type annotation.

from __future__ import annotations

import dataclasses
from typing import List, Optional

from torch.onnx._internal.diagnostics.infra.sarif_om import (
    _message,
    _property_bag,
    _stack_frame,
)


@dataclasses.dataclass
class Stack(object):
    """A call stack that is relevant to a result."""

    frames: List[_stack_frame.StackFrame] = dataclasses.field(
        metadata={"schema_property_name": "frames"}
    )
    message: Optional[_message.Message] = dataclasses.field(
        default=None, metadata={"schema_property_name": "message"}
    )
    properties: Optional[_property_bag.PropertyBag] = dataclasses.field(
        default=None, metadata={"schema_property_name": "properties"}
    )


# flake8: noqa

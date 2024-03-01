import sys
import lldb


class VariantSyntheticProvider:
    def __init__(self, valobj: lldb.SBValue, internal_dict):
        # Save the value object
        self.valobj = valobj

        # Placeholder structures mapping to the libstdc++ implementation
        _M_u: lldb.SBValue = valobj.GetChildMemberWithName("_M_u")
        _M_index: lldb.SBValue = valobj.GetChildMemberWithName("_M_index")
        _M_rest: lldb.SBValue = _M_u

        # Grab an array of the variants now
        nVariants: int = 0
        aData: list[lldb.SBValue] = []
        selectedVariant: int = _M_index.GetValueAsUnsigned()
        while _M_rest.GetNumChildren() > 0:
            _M_first = _M_rest.GetChildMemberWithName("_M_first")
            _M_storage = _M_first.GetChildMemberWithName("_M_storage")
            _M_rest = _M_rest.GetChildMemberWithName("_M_rest")
            nVariants += 1
            aData.append(_M_storage)

        # Now we create an array of the children
        self.children = [
            valobj.CreateValueFromData(
                "Index",
                lldb.SBData.CreateDataFromInt(selectedVariant),
                valobj.GetType().GetBasicType(lldb.eBasicTypeInt),
            ),
            valobj.CreateValueFromData(
                "Length",
                lldb.SBData.CreateDataFromInt(nVariants),
                valobj.GetType().GetBasicType(lldb.eBasicTypeInt),
            ),
            *[
                valobj.CreateValueFromData(
                    f"[{i}] {data.GetType().GetName()}", data.GetData(), data.GetType()
                )
                for i, data in enumerate(aData)
            ],
        ]

    def num_children(self):
        # this call should return the number of children that you want your object to have
        return len(self.children)

    def get_child_index(self, name):
        # this call should return the index of the synthetic child whose name
        # is given as argument
        return 0

    def get_child_at_index(self, index) -> lldb.SBValue:
        # this call should return a new LLDB SBValue object representing the
        # child at the index given as argument
        return self.children[index]

    def update(self):
        # this call should be used to update the internal state of this Python
        # object whenever the state of the variables in LLDB changes.[1]
        # Also, this method is invoked before any other method in the interface.
        return True

    def has_children(self):
        # this call should return True if this object might have children,
        # and False if this object can be guaranteed not to have children.[2]
        return True

    def get_value(self):
        # this call can return an SBValue to be presented as the value of the
        # synthetic value under consideration.[3]
        return lldb.SBValue("")


class ExprNodeListSyntheticProvider:
    def __init__(self, valobj: lldb.SBValue, internal_dict):
        # Grab the head_ and size_
        head_: lldb.SBValue = valobj.GetChildMemberWithName("head_")
        size_: lldb.SBValue = valobj.GetChildMemberWithName("size_")
        # Get the elements, prevent infinite loops by setting limit
        # Infinite/large sizes are due to uninitialized memory
        elements: list[lldb.SBValue] = []
        cur = head_
        limit = 1000
        if size_.GetValueAsUnsigned() < limit:
            for _ in range(0, min(size_.GetValueAsUnsigned(), limit)):
                elements.append(cur)
                cur = cur.GetChildMemberWithName("next_")
        # Create the children
        self.children = [
            valobj.CreateValueFromData("Size", size_.GetData(), size_.GetType()),
            *[
                valobj.CreateValueFromData(
                    f"[{i}] {el.GetType().GetName()}", el.GetData(), el.GetType()
                )
                for i, el in enumerate(elements)
            ],
        ]

    def num_children(self):
        return len(self.children)

    def get_child_index(self, name):
        return 0

    def get_child_at_index(self, index) -> lldb.SBValue:
        return self.children[index]

    def update(self):
        return True

    def has_children(self):
        return True

    def get_value(self):
        return lldb.SBValue("")


class SourceFileSyntheticProvider:
    def __init__(self, valobj: lldb.SBValue, internal_dict):
        # Grab the id_
        id_: lldb.SBValue = valobj.GetChildMemberWithName("id_")
        # Cast the id_ to a SourceManager::File*
        file: lldb.SBValue = valobj.CreateValueFromExpression(
            "File", f"*(SourceManager::File*){id_.GetValueAsUnsigned()}"
        )
        # Grab the file name
        file_name: lldb.SBValue = file.GetChildMemberWithName("name")
        self.children = [
            id_,
            valobj.CreateValueFromData(
                "File Name", file_name.GetData(), file_name.GetType()
            ),
        ]

    def num_children(self):
        return len(self.children)

    def get_child_index(self, name):
        return 1

    def get_child_at_index(self, index) -> lldb.SBValue:
        return self.children[index]

    def update(self):
        return True

    def has_children(self):
        return True

    def get_value(self):
        return lldb.SBValue("")


def SourceFileSummary(valobj: lldb.SBValue, internal_dict):
    return valobj.GetChildAtIndex(1).GetSummary().strip('"')


def SourceLocationSummary(valobj: lldb.SBValue, internal_dict):
    file = valobj.GetChildAtIndex(0).GetSummary()
    line = valobj.GetChildAtIndex(1).GetValueAsUnsigned()
    col = valobj.GetChildAtIndex(2).GetValueAsUnsigned()
    return f"{file}:{line}:{col}"


def __lldb_init_module(debugger: lldb.SBDebugger, dict):
    print("Loading std::variant printer")
    # lldb.formatters.Logger._lldb_formatters_debug_level = 2
    # https://github.com/sudara/melatonin_audio_sparklines/blob/main/sparklines.py#L133
    debugger.HandleCommand(
        'type synthetic add -x "^__gnu_cxx::__alloc_traits<std::allocator<std::variant<" --python-class stdvariant.VariantSyntheticProvider -w cplusplus'
    )
    debugger.HandleCommand(
        'type synthetic add -x "^std::__detail::__variant::_Variant_base<" --python-class stdvariant.VariantSyntheticProvider -w cplusplus'
    )
    debugger.HandleCommand(
        'type synthetic add -x "^ast::ExprNodeList$" --python-class stdvariant.ExprNodeListSyntheticProvider -w cplusplus'
    )
    debugger.HandleCommand(
        'type synthetic add -x "^SourceFile$" --python-class stdvariant.SourceFileSyntheticProvider -w cplusplus'
    )
    debugger.HandleCommand(
        'type summary add -x "^SourceFile$" -F stdvariant.SourceFileSummary -w cplusplus'
    )
    debugger.HandleCommand(
        'type summary add -x "^SourceLocation$" -F stdvariant.SourceLocationSummary -w cplusplus'
    )

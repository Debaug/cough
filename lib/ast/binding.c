#include "ast/binding.h"

IMPL_ARRAY_BUF(BindingId);
IMPL_ARRAY_BUF(TypeBindingEntry);
IMPL_ARRAY_BUF(ValueBindingEntry);
IMPL_ARRAY_BUF(Scope);

#define BINDING_ID_INDEX_MASK (usize)((usize)(-1) >> 1)
// 1 for values, 0 for types
#define BINDING_ID_KIND_MASK (usize)(~BINDING_ID_INDEX_MASK)

BindingRegistry binding_registry_new(void) {
    Scope root = {
        ._parent = -1,
        ._unordered_bindings = array_buf_new(BindingId)(),
        ._sequential_bindings = array_buf_new(BindingId)(),
    };
    ArrayBuf(Scope) scopes = array_buf_new(Scope)();
    array_buf_push(Scope)(&scopes, root);
    return (BindingRegistry){
        ._scopes = scopes,
        ._type_bindings = array_buf_new(TypeBindingEntry)(),
        ._value_bindings = array_buf_new(ValueBindingEntry)(),
    };
}

void binding_registry_free(BindingRegistry* registry) {
    for (usize i = 0; i < registry->_scopes.len; i++) {
        Scope* scope = &registry->_scopes.data[i];
        array_buf_free(BindingId)(&scope->_unordered_bindings);
        array_buf_free(BindingId)(&scope->_sequential_bindings);
    }
    array_buf_free(TypeBindingEntry)(&registry->_type_bindings);
    array_buf_free(ValueBindingEntry)(&registry->_value_bindings);
}

static bool find_binding_in_scope(
    BindingRegistry registry,
    ScopeLocation location,
    String name,
    BindingId* dst
) {
    Scope scope = registry._scopes.data[location.scope_id];
    for (usize i = 0; i < scope._unordered_bindings.len; i++) {
        BindingId id = scope._unordered_bindings.data[i];
        Binding binding = get_binding(registry, id);
        String binding_name;
        switch (binding.kind) {
        case BINDING_TYPE: binding_name = binding.as.type.name; break;
        case BINDING_VALUE: binding_name = binding.as.value.name; break;
        }
        if (eq(String)(name, binding_name)) {
            *dst = id;
            return true;
        }
    }

    if (location._pos > scope._sequential_bindings.len) {
        location._pos = scope._sequential_bindings.len;
    }
    for (isize i = location._pos - 1; i >= 0; i--) {
        BindingId id = scope._sequential_bindings.data[i];
        Binding binding = get_binding(registry, id);
        String binding_name;
        switch (binding.kind) {
        case BINDING_TYPE: binding_name = binding.as.type.name; break;
        case BINDING_VALUE: binding_name = binding.as.value.name; break;
        }
        if (eq(String)(name, binding_name)) {
            *dst = id;
            return true;
        }
    }
    return false;
}

bool find_binding(
    BindingRegistry registry,
    ScopeLocation location,
    String name,
    BindingId* dst
) {
    while (location.scope_id != -1) {
        Scope scope = registry._scopes.data[location.scope_id];
        if (find_binding_in_scope(registry, location, name, dst)) {
            return true;
        }
        location = scope._parent;
    }
    return false;
}

Binding get_binding(BindingRegistry registry, BindingId id) {
    usize index = id & BINDING_ID_INDEX_MASK;
    if (id & BINDING_ID_KIND_MASK) {
        ValueBindingEntry entry = registry._value_bindings.data[index];
        return (Binding){
            .id = id,
            .location = entry._location,
            .kind = BINDING_VALUE,
            .as.value = entry._data,
        };
    } else {
        TypeBindingEntry entry = registry._type_bindings.data[index];
        return (Binding){
            .id = id,
            .location = entry._location,
            .kind = BINDING_TYPE,
            .as.type = entry._data,
        };
    }
}

BindingMut get_binding_mut(BindingRegistry* registry, BindingId id) {
    usize index = id & BINDING_ID_INDEX_MASK;
    if (id & BINDING_ID_KIND_MASK) {
        ValueBindingEntry* entry = registry->_value_bindings.data + index;
        return (BindingMut){
            .id = id,
            .location = entry->_location,
            .kind = BINDING_VALUE,
            .as.value = &entry->_data,
        };
    } else {
        TypeBindingEntry* entry = registry->_type_bindings.data + index;
        return (BindingMut){
            .id = id,
            .location = entry->_location,
            .kind = BINDING_TYPE,
            .as.type = &entry->_data,
        };
    }
}

ScopeLocation scope_new(BindingRegistry* registry, ScopeLocation parent) {
    Scope scope = {
        ._parent = parent,
        ._unordered_bindings = array_buf_new(BindingId)(),
        ._sequential_bindings = array_buf_new(BindingId)(),
    };
    ScopeLocation location = {
        .scope_id = registry->_scopes.len,
        ._pos = 0,
    };
    array_buf_push(Scope)(&registry->_scopes, scope);
    return location;
}

ScopeLocation scope_end_location(BindingRegistry registry, ScopeId scope) {
    usize pos = registry._scopes.data[scope]._sequential_bindings.len;
    return (ScopeLocation){ .scope_id = scope, ._pos = pos };
}

static bool alloc_binding_entry(
    BindingRegistry* registry,
    ScopeLocation location,
    String name,
    bool sequential,
    BindingKind kind,
    BindingMut* dst
) {
    BindingId found_id;
    if (find_binding_in_scope(*registry, location, name, &found_id)) {
        *dst = get_binding_mut(registry, found_id);
        return false;
    }

    Scope* scope = &registry->_scopes.data[location.scope_id];
    BindingId id;
    usize idx;
    switch (kind) {
    case BINDING_TYPE:
        idx = registry->_type_bindings.len;
        id = idx;
        array_buf_push(TypeBindingEntry)(&registry->_type_bindings, (TypeBindingEntry){});
        *dst = (BindingMut){
            .id = id,
            .kind = BINDING_TYPE,
            .as.type = &registry->_type_bindings.data[idx]._data,
        };
        break;
    case BINDING_VALUE:
        idx = registry->_value_bindings.len;
        id = idx | BINDING_ID_KIND_MASK;
        array_buf_push(ValueBindingEntry)(&registry->_value_bindings, (ValueBindingEntry){});
        *dst = (BindingMut){
            .id = id,
            .kind = BINDING_VALUE,
            .as.value = &registry->_value_bindings.data[idx]._data,
        };
    }

    if (sequential) {
        array_buf_push(BindingId)(&scope->_sequential_bindings, id);
        dst->location = (ScopeLocation){ .scope_id = location.scope_id, ._pos = idx };
    } else {
        array_buf_push(BindingId)(&scope->_unordered_bindings, id);
        dst->location = (ScopeLocation){ .scope_id = location.scope_id, ._pos = 0 };
    }
    return true;
}

bool insert_type_binding(
    BindingRegistry* registry,
    ScopeId scope,
    TypeBinding binding,
    BindingMut* dst
) {
    BindingMut slot;
    ScopeLocation location = { .scope_id = scope, ._pos = -1 };
    if (!alloc_binding_entry(
        registry,
        location,
        binding.name,
        false,
        BINDING_TYPE,
        &slot
    )) {
        return false;
    }
    *slot.as.type = binding;
    if (dst) *dst = slot;
    return true;
}

bool insert_value_binding(
    BindingRegistry* registry,
    ScopeId scope,
    ValueBinding binding,
    BindingMut* dst
) {
    BindingMut slot;
    ScopeLocation location = { .scope_id = scope, ._pos = -1 };
    if (!alloc_binding_entry(
        registry,
        location,
        binding.name,
        false,
        BINDING_VALUE,
        &slot
    )) {
        return false;
    }
    *slot.as.value = binding;
    if (dst) *dst = slot;
    return true;
}

bool push_value_binding(
    BindingRegistry* registry,
    ScopeLocation* location,
    ValueBinding binding,
    BindingMut* dst
) {
    BindingMut slot;
    if (!alloc_binding_entry(
        registry,
        *location,
        binding.name,
        true,
        BINDING_VALUE,
        &slot
    )) {
        return false;
    }
    *slot.as.value = binding;
    if (dst) *dst = slot;
    location->_pos++;
    return true;
}

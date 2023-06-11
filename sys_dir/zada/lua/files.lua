---------------------------------------
--  profan's file system lua module  --
---------------------------------------

local calls = require("calls")
local syscalls = calls.syscalls

------------------------
-- Internal functions --
------------------------

local function copy_string_to_memory(str)
    local ptr = calls.malloc(#str + 1)

    -- Copy string to memory
    for i = 0, #str - 1 do
        profan.memset(ptr + i, 1, str:byte(i + 1))
    end
    profan.memset(ptr + #str, 1, 0)

    return ptr
end

local function copy_memory_to_string(ptr, size)
    local str = ""

    for i = 0, size - 1 do
        local char = profan.memval(ptr + i, 1)
        if char == 0 then
            break
        end
        str = str .. string.char(char)
    end

    return str
end


local function path_to_id(path)
    local ptr = copy_string_to_memory(path)
    local file_id = profan.call_c(calls.get_syscall(syscalls.fs_path_to_id), 4, ptr)

    calls.free(ptr)
    return file_id
end


------------------------
--  Shared functions  --
------------------------

local function does_path_exist(path)
    local ptr = copy_string_to_memory(path)
    local exists = profan.call_c(calls.get_syscall(syscalls.fs_does_path_exists), 4, ptr)

    free(ptr)
    return exists ~= 0 -- cast to boolean
end


local function get_path_type(path)
    if not does_path_exist(path) then
        return nil
    end

    local ptr = copy_string_to_memory(path)
    local path_type = profan.call_c(calls.get_syscall(syscalls.get_sector_type), 4, path_to_id(path))

    calls.free(ptr)
    return path_type
end


local function get_file_size(file_name)
    if not does_path_exist(file_name) then
        return nil
    end

    if get_path_type(file_name) ~= 2 then
        return nil
    end

    local ptr = copy_string_to_memory(file_name)
    local size = profan.call_c(calls.get_syscall(syscalls.fs_get_file_size), 4, ptr)

    free(ptr)
    return size
end


local function get_dir_size(dir_name)
    if not does_path_exist(dir_name) then
        return nil
    end

    if get_path_type(dir_name) < 3 then
        return nil
    end

    local ptr = copy_string_to_memory(dir_name)
    local size = profan.call_c(calls.get_syscall(syscalls.fs_get_dir_size), 4, ptr)

    free(ptr)
    return size
end


local function get_element_name(file_id)
    local ptr = calls.malloc(256)
    profan.call_c(calls.get_syscall(syscalls.fs_get_element_name), 4, file_id, 4, ptr)

    local file_name = copy_memory_to_string(ptr, 256)
    calls.free(ptr)
    return file_name
end


local function get_dir_content(dir_name)
    if not does_path_exist(dir_name) then
        return nil
    end

    local ptr = copy_string_to_memory(dir_name)

    -- malloc output, a file index buffer
    local dir_size = get_dir_size(dir_name)
    local output = calls.malloc((dir_size + 1) * 4)

    profan.call_c(calls.get_syscall(syscalls.fs_get_dir_content), 4, ptr, 4, output)

    local content = {}
    
    for i = 0, dir_size - 1 do
        local file_id = profan.memval(output + i * 4, 4)
        local file_name = get_element_name(file_id)
        table.insert(content, file_name)
    end

    calls.free(ptr)
    calls.free(output)
    return content
end


local function get_file_content(file_name)
    if not does_path_exist(file_name) then
        return nil
    end

    if get_path_type(file_name) ~= 2 then
        return nil
    end

    local file_size = get_file_size(file_name)
    local ptr = copy_string_to_memory(file_name)

    -- malloc output, a char buffer
    local output = calls.malloc(file_size + 1)

    profan.call_c(calls.get_syscall(syscalls.fs_read_file), 4, ptr, 4, output)

    local content = copy_memory_to_string(output, file_size)

    calls.free(ptr)
    calls.free(output)
    return content
end

local function set_file_content(file_name, content)
    if not does_path_exist(file_name) then
        return false
    end

    if get_path_type(file_name) ~= 2 then
        return false
    end

    -- chek if content is a string
    if type(content) ~= "string" then
        return false
    end

    local name_ptr = copy_string_to_memory(file_name)
    local content_ptr = copy_string_to_memory(content)

    profan.call_c(calls.get_syscall(syscalls.fs_write_in_file), 4, name_ptr, 4, content_ptr, 4, #content)

    calls.free(name_ptr)
    calls.free(content_ptr)
    return true
end

local function create_element(element, elm_type)
    if does_path_exist(element) then
        return false
    end

    local parent_dir = element:match("(.+)/") or "/"
    local elm_name = ("asqel" .. element):match(".+/(.+)")

    if not parent_dir or not elm_name then
        return false
    end

    -- check if parent dir exists
    if not does_path_exist(parent_dir) then
        return false
    end

    -- check if parent dir is a directory
    if get_path_type(parent_dir) < 3 then
        return false
    end

    local parent_dir_ptr = copy_string_to_memory(parent_dir)
    local elm_name_ptr = copy_string_to_memory(elm_name)
    
    if elm_type == 2 then
        profan.call_c(calls.get_syscall(syscalls.fs_make_file), 4, parent_dir_ptr, 4, elm_name_ptr)
    else    
        profan.call_c(calls.get_syscall(syscalls.fs_make_dir), 4, parent_dir_ptr, 4, elm_name_ptr)
    end

    calls.free(parent_dir_ptr)
    calls.free(elm_name_ptr)
    return true
end


local function create_file(file_name)
    return create_element(file_name, 2)
end


local function create_dir(dir_name)
    return create_element(dir_name, 3)
end


return {
    does_path_exist = does_path_exist,

    get_file_size = get_file_size,
    get_dir_size = get_dir_size,

    get_dir_content = get_dir_content,
    get_element_name = get_element_name,
    get_path_type = get_path_type,

    get_file_content = get_file_content,
    set_file_content = set_file_content,

    create_file = create_file,
    create_dir = create_dir,
}

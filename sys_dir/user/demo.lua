f = require("files")

path = "/zada/"

for k, e in ipairs(f.get_dir_content(path)) do
    print("key:", k,
          "name:", e,
          "path:", path .. e,
          "cnt:", f.get_dir_size(path .. e),
          "size:", f.get_file_size(path .. e),
          "type:", f.get_path_type(path .. e)
    )
end

print("create:", f.create_dir("/test"))
print("create:", f.create_file("/test/mhh.txt"))

-- print("set: ", f.set_file_content("/user/README.txt", "Hello World!"))
-- print("get: ", f.get_file_content("/user/README.txt"))

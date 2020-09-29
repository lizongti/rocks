local package_path = {package.path, "deps/?.lua", "deps/?/init.lua"}
local package_cpath = {package.cpath}
package.path = table.concat(package_path, ";")
package.cpath = table.concat(package_cpath, ";")

os.exit(require("process").bootstrap(loadfile("./app/postgres.lua")))

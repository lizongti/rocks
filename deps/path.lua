local package_path = {
    package.path,
	".\\?.lua",
	".\\?\\init.lua",
	".\\deps.\\?.lua",
	".\\deps.\\?.\\init.lua",
	"C:\\Program Files\\LuaRocks\\5.1\\lua\\?.lua",
	"C:\\Program Files\\LuaRocks\\5.1\\lua\\?\\init.lua",
}
local package_cpath = {
	package.cpath,
	"C:\\Program Files\\LuaRocks\\5.1\\clibs\\?.dll",
    -- string.gsub(package.cpath, "%?", "clibs\\?")
}

package.path = table.concat(package_path, ";")
package.cpath = table.concat(package_cpath, ";")
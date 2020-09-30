local _, openssl = pcall(require, 'openssl')

local function randomBytesOpenSSL(size, callback)
    local str = openssl.random(size)
    if callback then callback(nil, str) end
    return str
end

local function randomBytesInsecure(size, callback)
    print('**** WARNING: Using insecure RNG ****')
    local str = {}
    for i = 0, size do table.insert(str, math.random()) end
    str = table.concat(str)
    if callback then callback(str) end
    return str
end

if type(openssl) == 'table' then
    return {randomBytes = randomBytesOpenSSL}
else
    return {randomBytes = randomBytesInsecure}
end

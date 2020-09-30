local mq = require "queue"
print("pop", mq.pop("receiver"))
print("push", mq.push("receiver", "first"))
print("push", mq.push("receiver", "first"))

for i = 1, 1021 do
    mq.push("receiver", "first")
end

print("push", mq.push("receiver", "first"))
print("push", mq.push("receiver", "first"))

print("pop", mq.pop("receiver"))
print("pop", mq.pop("receiver"))

for i = 1, 1021 do
    mq.pop("receiver")
end

print("pop", mq.pop("receiver"))
print("pop", mq.pop("receiver"))

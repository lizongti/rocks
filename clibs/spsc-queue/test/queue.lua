local mq = require "spsc_queue"

print("push", mq.push("receiver", "sender", "first"))
print("push", mq.push("receiver", "sender", "first"))

for i = 1, 1021 do
    mq.push("receiver", "sender", "first")
end

print("push", mq.push("receiver", "sender", "first"))
print("push", mq.push("receiver", "sender", "first"))

print("pop", mq.pop("receiver", "sender"))
print("pop", mq.pop("receiver", "sender"))

for i = 1, 1021 do
    mq.pop("receiver", "sender")
end

mq.flush("receiver", "sender")

print("pop", mq.pop("receiver", "sender"))
print("pop", mq.pop("receiver", "sender"))

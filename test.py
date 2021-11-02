import sys
f = open(sys.argv[1])

nums = []
for l in f.readlines():
    l = l.strip()
    n = int(l)
    nums.append(n)

d = {}
for i in range(1, len(nums)):
    diff = nums[i] - nums[i-1]
    if diff not in d.keys():
        d[diff] = 0
    else:
        d[diff] += 1

for k in d.keys():
    print(str(k) + " " + str(d[k]))

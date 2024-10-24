buf = {}
with open("./skk/SKK-JISYO.ML.txt", "r", encoding="utf8") as f:
    for line in f:
        if line[0] == ";":
            continue
        first = line[0].encode("utf8").hex()
        buf.setdefault(first, []).append(line)
for k, v in buf.items():
    print(k, v)
    with open("dic/" + k + ".txt", "w", encoding="utf8") as f:
        for l in v:
            f.write(l)

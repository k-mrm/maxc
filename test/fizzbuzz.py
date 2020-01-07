n = 600000
i = 1

while i <= n:
    if i % 15 == 0: print("fizzbuzz")
    elif i % 3 == 0: print("fizz")
    elif i % 5 == 0: print("buzz")
    else: print(i)

    i += 1

def f():
    main=open("A.txt","r")
    v=[]
    for i in range(5):
        v.append([])
    for l in main.readlines():
        print(l)
        f=l.split("-")
        index=int(f[0])
        v[index].append(f[1][0])
    print(v)
    for i in range(5):
        file_actual = open(f"B{i}.txt","r")
        j=0
        for line in file_actual.readlines():
            if v[i][j] not in line:
                print(f"error in file B{i} at line {j}")
                break;
            j=j+1
        file_actual.close()
f()

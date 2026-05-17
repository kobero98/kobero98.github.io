import random
import string

NUM_LINES = 1000         # Numero di righe da generare in A.txt
NUM_BUCKETS = 5          # Numero di file B (da B0 a B4)
INPUT_FILE = "A.txt"
OUTPUT_FILES = [f"B{i}.txt" for i in range(NUM_BUCKETS)]

# Apre tutti i file di output B in scrittura
output_handles = [open(fname, "w") for fname in OUTPUT_FILES]

# Apre il file A.txt in scrittura
with open(INPUT_FILE, "w") as a_file:
    for _ in range(NUM_LINES):
        index = random.randint(0, NUM_BUCKETS - 1)
        letter = random.choice(string.ascii_uppercase)
        line = f"{index}-{letter}\n"
        a_file.write(line)
        output_handles[index].write(f"{letter}\n")

# Chiude tutti i file B*
for f in output_handles:
    f.close()

print(f"Generati {NUM_LINES} elementi in '{INPUT_FILE}' e file di output {', '.join(OUTPUT_FILES)}.")


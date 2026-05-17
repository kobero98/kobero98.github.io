import csv
import random
import sys

nomi = [
    "Mario", "Luca", "Anna", "Giulia", "Paolo",
    "Francesca", "Marco", "Sara", "Davide", "Elena"
]

cognomi = [
    "Rossi", "Bianchi", "Verdi", "Neri", "Gialli",
    "Romano", "Costa", "Greco", "Ferrari", "Conti"
]

def genera_persona():
    nome = random.choice(nomi)
    cognome = random.choice(cognomi)

    eta = random.randint(10, 80)

    altezza = round(random.uniform(1.50, 2.00), 2)

    return [nome, cognome, eta, altezza]


def main():

    if len(sys.argv) != 3:
        print(f"Uso: python {sys.argv[0]} <numero_entry> <output.csv>")
        sys.exit(1)

    numero_entry = int(sys.argv[1])
    output_file = sys.argv[2]

    with open(output_file, "w", newline="") as csvfile:

        writer = csv.writer(csvfile)

        for _ in range(numero_entry):
            writer.writerow(genera_persona())

    print(f"Creato file '{output_file}' con {numero_entry} entry.")


if __name__ == "__main__":
    main()

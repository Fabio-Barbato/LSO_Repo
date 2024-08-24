# Usa una piccola immagine di base che include gli strumenti necessari per compilare il codice C
FROM gcc:latest

# Crea una directory di lavoro all'interno del container
WORKDIR /usr/src/app

# Copia il codice del server nella directory di lavoro del container
COPY . .

# Compila il server con il flag per abilitare strptime
RUN gcc -D_XOPEN_SOURCE=500 -o server server.c Controller/cJSON.c -lm

# Esponi la porta 8080 in modo che possa essere accessibile dall'esterno
EXPOSE 8080

# Esegui il server quando il container viene avviato
CMD ["./server"]
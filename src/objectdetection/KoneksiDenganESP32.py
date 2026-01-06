import socket
import subprocess

HOST = '192.168.95.79' 
PORT = 5000

PYTHON_PATH = "d:/Kuliah/TUGAS AKHIR/KODINGAN/Python/ProgramDeteksiTelur.py"
def run_detection():
    print("Trigger diterima! Menjalankan YOLO...")
    subprocess.Popen(["python", PYTHON_PATH], shell=True) 

def mark_done():
    print("Proses selesai! Menunggu trigger berikutnya...")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f"[SERVER] Menunggu koneksi di {HOST}:{PORT}...")
    while True:
        conn, addr = s.accept()
        with conn:
            data = conn.recv(1024).decode()
            if "TRIGGER" in data:
                run_detection()
            elif "DONE" in data:
                mark_done()
            else:
                print("[WARNING] Perintah tidak dikenali:", data)


/*----------------------------
  Nama File : lat201.java
  Author    : Munir
  ---------------------------*/
import java.io.*;

class lat201 {
    public static void main(String[] args) {
        // deklarasi variabel
        String kata = "";
        boolean akhir = false;
        int huruf;

        // memberi tahu pengguna untuk memasukkan input
        System.out.print("Masukkan kata Anda: ");

        while (!akhir) {
            try {
                // membaca karakter satu per satu
                huruf = System.in.read();
                if (huruf != -1 && huruf != '\n') {
                    kata = kata + (char) huruf;
                } else if (huruf == '\n') {
                    akhir = true; // akhir input jika tekan Enter
                }
            } catch (IOException e) {
                System.err.println("Mengalami kesalahan");
                akhir = true;
            }
        }

        System.out.println("Kalimat yang Anda ketik: " + kata);
    }
}

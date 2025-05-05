
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <limits>

using namespace std;

// Struct untuk merepresentasikan Taksi
struct Taksi {
    int id;
    char lokasiSaatIni;
    bool bebas;
    double totalPendapatan;

    Taksi(int id) : id(id), lokasiSaatIni('A'), bebas(true), totalPendapatan(0) {}
};

// Struct untuk merepresentasikan Pemesanan
struct Pemesanan {
    int idPemesanan;
    char titikPenjemputan;
    char titikPenurunan;
    double tarif;

    Pemesanan(int id, char penjemputan, char penurunan, double tarif)
        : idPemesanan(id), titikPenjemputan(penjemputan), titikPenurunan(penurunan), tarif(tarif) {}
};

// Fungsi untuk menghitung jarak antar titik
int hitungJarak(char titik1, char titik2) {
    return abs(titik2 - titik1) * 15; // Setiap titik berjarak 15 km
}

// Fungsi untuk menghitung tarif
double hitungTarif(int jarak) {
    const double tarifDasar = 100; // Tarif minimum untuk 5 km pertama
    const double tarifPerKm = 10; // Tarif tambahan per km setelah 5 km
    if (jarak <= 5) {
        return tarifDasar;
    } else {
        return tarifDasar + (jarak - 5) * tarifPerKm;
    }
}

// Fungsi untuk menemukan taksi terdekat yang tersedia
int cariTaksiTerdekat(vector<Taksi>& taksi, char titikPenjemputan) {
    int indeksTaksiTerdekat = -1;
    int jarakMinimal = numeric_limits<int>::max();
    double pendapatanMinimal = numeric_limits<double>::max();

    for (int i = 0; i < taksi.size(); i++) {
        if (taksi[i].bebas) {
            int jarak = hitungJarak(taksi[i].lokasiSaatIni, titikPenjemputan);
            if (jarak < jarakMinimal || (jarak == jarakMinimal && taksi[i].totalPendapatan < pendapatanMinimal)) {
                indeksTaksiTerdekat = i;
                jarakMinimal = jarak;
                pendapatanMinimal = taksi[i].totalPendapatan;
            }
        }
    }
    return indeksTaksiTerdekat;
}

int main() {
    vector<Taksi> daftarTaksi = {Taksi(1), Taksi(2), Taksi(3), Taksi(4)};
    vector<Pemesanan> daftarPemesanan;
    int nomorPemesanan = 1;

    while (true) {
        cout << "\n=== Aplikasi Pemesanan Taksi ===\n";
        cout << "1. Pesan Taksi\n";
        cout << "2. Lihat Status Taksi\n";
        cout << "3. Keluar\n";
        cout << "Masukkan pilihan Anda: ";
        int pilihan;
        cin >> pilihan;

        if (pilihan == 1) {
            char titikPenjemputan, titikPenurunan;
            cout << "Masukkan Titik Penjemputan (A-F): ";
            cin >> titikPenjemputan;
            cout << "Masukkan Titik Penurunan (A-F): ";
            cin >> titikPenurunan;

            if (titikPenjemputan < 'A' || titikPenjemputan > 'F' || titikPenurunan < 'A' || titikPenurunan > 'F') {
                cout << "Titik tidak valid! Masukkan titik antara A dan F.\n";
                continue;
            }

            int jarak = hitungJarak(titikPenjemputan, titikPenurunan);
            double tarif = hitungTarif(jarak);

            int indeksTaksi = cariTaksiTerdekat(daftarTaksi, titikPenjemputan);
            if (indeksTaksi == -1) {
                cout << "Tidak ada taksi yang tersedia saat ini. Pemesanan ditolak.\n";
            } else {
                daftarTaksi[indeksTaksi].bebas = false;
                daftarTaksi[indeksTaksi].lokasiSaatIni = titikPenurunan;
                daftarTaksi[indeksTaksi].totalPendapatan += tarif;

                daftarPemesanan.emplace_back(nomorPemesanan++, titikPenjemputan, titikPenurunan, tarif);
                cout << "Taksi nomor " << daftarTaksi[indeksTaksi].id << " segera datang\n";
                cout << "Tarif: Rp. " << tarif << "\n";
            }
        } else if (pilihan == 2) {
            cout << "\n=== Status Taksi ===\n";
            for (const auto& taksi : daftarTaksi) {
                cout << "Taksi " << taksi.id << " - Lokasi: " << taksi.lokasiSaatIni
                     << ", Status: " << (taksi.bebas ? "Bebas" : "Sibuk")
                     << ", Total Pendapatan: Rp. " << taksi.totalPendapatan << "\n";
            }
        } else if (pilihan == 3) {
            cout << "Keluar dari aplikasi. Terima kasih!\n";
            break;
        } else {
            cout << "Pilihan tidak valid! Silakan coba lagi.\n";
        }
    }

    return 0;
}

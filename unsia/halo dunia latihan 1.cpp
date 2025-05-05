
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

struct Penduduk {
    std::string noKK;
    std::vector<std::string> anggota;
};

void cetakPenduduk(Penduduk penduduk, int tingkat = 0) {
    for (int i = 0; i < tingkat; i++) {
        std::cout << " ";
    }
    std::cout << "No.KK: " << penduduk.noKK << std::endl;
    for (std::string anggota : penduduk.anggota) {
        for (int i = 0; i < tingkat + 1; i++) {
            std::cout << " ";
        }
        std::cout << "- " << anggota << std::endl;
    }
}

void simpanDataKeFile(std::vector<Penduduk> penduduk) {
    std::ofstream file("data_penduduk.txt", std::ios_base::app);
    if (file.is_open()) {
        for (int i = 0; i < penduduk.size(); i++) {
            file << "data masarakat" << std::endl;
            file << "No.KK: " << penduduk[i].noKK << std::endl;
            for (std::string anggota : penduduk[i].anggota) {
                file << "- " << anggota << std::endl;
            }
            file << std::endl;
        }
        file.close();
        std::cout << "Data berhasil disimpan ke file data_penduduk.txt" << std::endl;
    } else {
        std::cout << "Gagal membuka file" << std::endl;
    }
}

int main() {
    char lagi = 'Y';
    std::vector<Penduduk> penduduk;
    while (lagi == 'Y' || lagi == 'y') {
        Penduduk temp;
        std::cout << "Masukkan No.KK : ";
        std::cin >> temp.noKK;
        int jumlahAnggota;
        std::cout << "Jumlah anggota : ";
        std::cin >> jumlahAnggota;
        for (int i = 0; i < jumlahAnggota; i++) {
            std::string nama;
            std::cout << "Nama anggota " << i + 1 << ": ";
            std::cin >> nama;
            temp.anggota.push_back(nama);
        }
        penduduk.push_back(temp);
        std::cout << "Berhasil disimpan" << std::endl << std::endl;
        std::cout << "Lanjut input data?" << std::endl;
        std::cout << " Y / N" << std::endl;
        std::cin >> lagi;
    }
    simpanDataKeFile(penduduk);
    std::cout << "\nData Masyarakat Rt01 Rw03" << std::endl;
    std::cout << "Surat Tugas Desa No DD.XI/01022025" << std::endl;
    for (int i = 0; i < penduduk.size(); i++) {
        cetakPenduduk(penduduk[i]);
    }
    return 0;
}

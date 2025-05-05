
// UAS 23 februari 2025 ================================================================================================================
#include <iostream>  
#include <string>  
#include <vector>  

using namespace std;  

// Struktur untuk menyimpan informasi buku  
struct Buku {  
    string judul;  
    string pengarang;  
    int jumlah;  
};  

int main() {  
    vector<Buku> daftarBuku;  
    char lagi;  

    // Bagian A: Memasukkan data buku  
    do {  
        Buku buku;  

        cout << "Judul buku\t: ";  
        getline(cin, buku.judul); // Menggunakan getline agar bisa membaca spasi  

        cout << "Nama pengarang\t: ";  
        getline(cin, buku.pengarang);  

        cout << "Jumlah\t\t: ";  
        cin >> buku.jumlah;  
        cin.ignore(); // Membersihkan newline dari buffer  

        daftarBuku.push_back(buku);  

        cout << "Mau merekam data lagi [Y/T] ? ";  
        cin >> lagi;  
        cin.ignore(); // Membersihkan newline dari buffer  

    } while (toupper(lagi) == 'Y');  

    // Bagian B: Menampilkan data buku  
    cout << "\nNo\tJudul Buku\t\tNama Pengarang\t\tJumlah" << endl;  
    for (int i = 0; i < daftarBuku.size(); i++) {  
        cout << i + 1 << ".\t" << daftarBuku[i].judul << "\t\t"  
             << daftarBuku[i].pengarang << "\t\t" << daftarBuku[i].jumlah << endl;  
    }  
    cin.get();// menuggu input dari user sebelum program berahir
    return 0;  
}

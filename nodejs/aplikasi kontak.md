# Aplikasi Kontak Sederhana dengan Node.js, Express, EJS, dan MySQL/MariaDB

Aplikasi ini adalah contoh dasar dari aplikasi manajemen kontak sederhana yang dibangun menggunakan teknologi web modern: Node.js sebagai runtime JavaScript, Express.js sebagai kerangka kerja web, EJS (Embedded JavaScript) sebagai template engine, dan MySQL/MariaDB sebagai sistem manajemen database relasional.

Aplikasi ini mencakup fungsionalitas CRUD (Create, Read, Update, Delete) untuk mengelola data kontak.

## Fitur

* **Daftar Kontak:** Menampilkan semua kontak yang tersimpan di database.
* **Detail Kontak:** Menampilkan informasi lengkap untuk satu kontak.
* **Tambah Kontak:** Formulir untuk menambahkan kontak baru ke database.
* **Edit Kontak:** Mengubah informasi kontak yang sudah ada.
* **Hapus Kontak:** Menghapus kontak dari database.
* **Penyimpanan Data Persisten:** Menggunakan MySQL/MariaDB untuk menyimpan data agar tidak hilang saat server di-restart.

## Teknologi yang Digunakan

* **Node.js:** Runtime JavaScript.
* **Express.js:** Kerangka kerja web minimalis dan fleksibel untuk Node.js.
* **EJS (Embedded JavaScript):** Template engine untuk menyisipkan JavaScript ke dalam HTML.
* **MySQL/MariaDB:** Sistem manajemen database relasional.
* **`mysql2`:** Driver MySQL/MariaDB untuk Node.js yang mendukung Promise.

## Persyaratan Sistem

Pastikan Anda memiliki hal-hal berikut terinstal di sistem Anda:

* **Node.js dan npm (Node Package Manager):** [https://nodejs.org/](https://nodejs.org/)
* **MySQL Server atau MariaDB Server:**
    * **Debian/Ubuntu:** `sudo apt install mysql-server` atau `sudo apt install mariadb-server`
    * Panduan instalasi untuk sistem operasi lain dapat ditemukan di dokumentasi resmi MySQL atau MariaDB.

## Langkah-langkah Instalasi dan Konfigurasi

Ikuti langkah-langkah di bawah ini secara berurutan.

### 1. Kloning Repositori (Jika dari GitHub/GitLab)

Jika Anda mendapatkan kode ini dari repositori Git, mulailah dengan mengkloningnya:

```bash
git clone <url_repositori_anda>
cd <nama_folder_aplikasi>
```

Jika Anda membuat dari awal, pastikan Anda berada di direktori proyek Anda.

### 2. Instal Dependensi Node.js

Navigasikan ke direktori proyek Anda di terminal dan instal semua dependensi yang diperlukan:

```bash
npm install express ejs mysql2
```

### 3. Instal dan Konfigurasi MySQL/MariaDB Server

Pastikan MySQL atau MariaDB Server Anda sudah terinstal dan berjalan.

a. Instalasi (Contoh untuk Debian 12)

```bash
sudo apt update
sudo apt install mysql-server # Atau mariadb-server
sudo systemctl start mysql    # Atau systemctl start mariadb
sudo systemctl enable mysql   # Atau systemctl enable mariadb
```

b. Pengamanan Instalasi MySQL/MariaDB (PENTING!)

Jalankan perintah ini untuk mengatur kata sandi root dan mengamankan instalasi Anda:

```bash
sudo mysql_secure_installation
```

Ikuti petunjuk di layar (atur kata sandi `root`, hapus pengguna anonim, larang login `root` jarak jauh, dll.).

c. Buat Database dan Tabel Kontak

Masuk ke MySQL/MariaDB Monitor sebagai `root` dengan kata sandi yang telah Anda atur:

```bash
mysql -u root -p
```

(Masukkan kata sandi saat diminta)

Setelah masuk, jalankan perintah SQL berikut untuk membuat database contact_app dan tabel contacts:

```bash
CREATE DATABASE contact_app;
USE contact_app;
CREATE TABLE contacts (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nama VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    telepon VARCHAR(20) NOT NULL
);
exit;
```

d. Buka Port MySQL/MariaDB di Firewall (Jika Menggunakan UFW)
Jika Anda menggunakan UFW (Uncomplicated Firewall) di Debian, Anda perlu membuka port 3306.

```bash
sudo ufw allow 3306/tcp
sudo ufw reload
```

### 4. Konfigurasi Aplikasi (File `app.js`)

Buka file `app.js` Anda dan pastikan konfigurasi koneksi database Anda benar.

```bash
// app.js

// ... (bagian atas file)

// --- Konfigurasi MySQL dengan mysql2 ---
const mysql = require('mysql2/promise'); // Impor versi Promise

let db; // Variabel untuk menyimpan koneksi database

async function connectToDatabase() {
    try {
        db = await mysql.createConnection({
            host: 'localhost', // Atau '127.0.0.1' jika ada masalah koneksi IPv6
            user: 'root',      // Pengguna database Anda
            password: 'YOUR_MYSQL_ROOT_PASSWORD', // GANTI DENGAN PASSWORD ROOT MYSQL ANDA
            database: 'contact_app'
        });
        console.log('Terkoneksi ke MySQL!');
    } catch (err) {
        console.error('Gagal koneksi ke MySQL:', err);
        process.exit(1); // Keluar dari aplikasi jika gagal koneksi database
    }
}
connectToDatabase(); // Panggil fungsi koneksi saat aplikasi dimulai
// ------------------------------------------

// ... (middleware dan rute lainnya)
```

PENTING: Pastikan Anda mengganti 'YOUR_MYSQL_ROOT_PASSWORD' dengan kata sandi root MySQL/MariaDB yang Anda atur di langkah 3b.

### 5. Siapkan Struktur Folder Views dan Public
   
Pastikan Anda memiliki struktur folder berikut di root proyek Anda:

```
.
├── app.js
├── package.json
├── node_modules/
├── public/
│   └── css/
│       └── style.css  (Untuk styling dasar)
└── views/
    ├── index.ejs
    ├── about.ejs
    ├── contact.ejs
    ├── detail.ejs
    ├── add-contact.ejs
    └── edit-contact.ejs
    └── 404.ejs
```

Contoh Isi File EJS (Views)

Pastikan file EJS Anda memiliki konten yang sesuai. Berikut adalah contoh singkat untuk `contact.ejs` dan `detail.ejs` yang penting untuk integrasi database.

`views/contact.ejs`
```HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title><%= title %></title>
    <link rel="stylesheet" href="/css/style.css">
</head>
<body>
    <h1><%= heading %></h1>
    <p><a href="/contact/add">Tambah Kontak Baru</a></p>

    <% if (contacts.length === 0) { %>
        <p>Belum ada kontak yang terdaftar.</p>
    <% } else { %>
        <table>
            <thead>
                <tr>
                    <th>Nama</th>
                    <th>Email</th>
                    <th>Telepon</th>
                </tr>
            </thead>
            <tbody>
                <% contacts.forEach(contact => { %>
                    <tr>
                        <td><a href="/contact/<%= contact.id %>"><%= contact.nama %></a></td>
                        <td><%= contact.email %></td>
                        <td><%= contact.telepon %></td>
                    </tr>
                <% }) %>
            </tbody>
        </table>
    <% } %>

    <a href="/">Kembali ke Beranda</a>
</body>
</html>
```


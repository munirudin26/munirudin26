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

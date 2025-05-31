# Mulai Proyek WMS
Berikut adalah langkah-langkah untuk memulai proyek Anda dan penjelasan singkat untuk setiap file atau folder yang Anda sebutkan:

1. Inisialisasi Proyek Node.js
Pertama, buat folder wms-app dan masuk ke dalamnya. Kemudian inisialisasi proyek Node.js Anda:

Bash

mkdir wms-app
cd wms-app
npm init -y
2. Buat Struktur Folder dan File
Selanjutnya, buatlah semua folder dan file sesuai dengan struktur yang Anda berikan. Anda bisa melakukannya secara manual atau menggunakan perintah mkdir dan touch di terminal:

Bash

# Buat folder utama
mkdir src views

# Buat folder di dalam src/
mkdir src/config src/controllers src/models src/routes src/middleware

# Buat folder di dalam views/
mkdir views/auth views/admin views/admin/users views/dashboard

# Buat file-file utama
touch src/app.js .env .env.example .gitignore package.json hash_password.js

# Buat file-file di src/config/
touch src/config/database.js

# Buat file-file di src/controllers/
touch src/controllers/authController.js src/controllers/userController.js

# Buat file-file di src/models/
touch src/models/User.js

# Buat file-file di src/routes/
touch src/routes/authRoutes.js src/routes/adminRoutes.js

# Buat file-file di src/middleware/
touch src/middleware/authMiddleware.js

# Buat file-file di views/auth/
touch views/auth/login.ejs

# Buat file-file di views/admin/
touch views/admin/users/list.ejs views/admin/users/create.ejs views/admin/users/edit.ejs
touch views/admin/change-password.ejs

# Buat file-file di views/dashboard/
touch views/dashboard/admin.ejs views/dashboard/ppic.ejs views/dashboard/packing.ejs views/dashboard/mixing.ejs views/dashboard/milling.ejs views/dashboard/qc.ejs views/dashboard/gudang.ejs

# Buat file error
touch views/error.ejs
3. Instal Dependensi yang Dibutuhkan
Anda akan memerlukan beberapa paket dasar untuk aplikasi Express.js dan EJS. Anda bisa menginstalnya dengan:

Bash

npm install express ejs pg dotenv bcryptjs jsonwebtoken
Penjelasan Singkat Dependensi:

express: Framework web untuk Node.js.
ejs: Templating engine untuk membuat tampilan dinamis.
pg: Klien PostgreSQL (jika Anda menggunakan PostgreSQL sebagai database, ganti sesuai database Anda, misalnya mysql2 untuk MySQL).
dotenv: Memuat variabel lingkungan dari file .env.
bcryptjs: Untuk hashing password.
jsonwebtoken: Untuk membuat dan memverifikasi JSON Web Tokens (JWT) untuk autentikasi.
Penjelasan Singkat Setiap Bagian
node_modules/: Folder ini akan dibuat secara otomatis oleh npm dan berisi semua dependensi yang diinstal.
src/: Folder utama untuk semua kode sumber aplikasi Anda.
config/database.js: File ini akan berisi konfigurasi koneksi database Anda (misalnya, kredensial database, pool koneksi).
controllers/: Berisi logika bisnis utama aplikasi. Setiap file *Controller.js akan menangani permintaan untuk fungsionalitas tertentu (misalnya, authController.js untuk login/register, userController.js untuk manajemen pengguna admin).
models/User.js: Mendefinisikan struktur data dan interaksi dengan tabel users di database. Anda akan menempatkan kueri SQL di sini.
routes/: Mendefinisikan semua API endpoint aplikasi Anda.
authRoutes.js: Rute untuk fungsi autentikasi (login, logout).
adminRoutes.js: Rute khusus yang hanya bisa diakses oleh pengguna dengan peran "admin" (misalnya, mengelola pengguna lain).
middleware/authMiddleware.js: Berisi fungsi-fungsi middleware kustom, seperti verifikasi token JWT untuk autentikasi dan pengecekan peran pengguna untuk otorisasi.
app.js: File utama yang akan menginisialisasi aplikasi Express.js, mengkonfigurasi middleware global, dan mengimpor serta menggunakan semua rute yang didefinisikan.
views/: Berisi semua file template EJS yang akan dirender sebagai halaman web.
auth/login.ejs: Template untuk halaman login.
admin/users/list.ejs, create.ejs, edit.ejs: Template untuk halaman manajemen pengguna oleh admin (daftar, tambah, edit).
admin/change-password.ejs: Template untuk form penggantian password bagi admin.
dashboard/: Template dashboard yang berbeda untuk setiap peran pengguna (admin, ppic, packing, dll.).
error.ejs: Halaman error generik.
.env: File ini berisi variabel lingkungan yang sensitif (misalnya, DATABASE_URL, JWT_SECRET). File ini tidak boleh di-commit ke repositori Git publik.
.env.example: Contoh template untuk file .env, menunjukkan variabel apa saja yang dibutuhkan tanpa nilainya.
.gitignore: Memberi tahu Git file atau folder mana yang harus diabaikan (misalnya, node_modules/, .env).
package.json: File manifest proyek Node.js yang berisi metadata proyek dan daftar dependensi.
package-lock.json: Mencatat versi spesifik dari setiap dependensi yang diinstal untuk memastikan instalasi yang konsisten di lingkungan yang berbeda.
hash_password.js: Utilitas sementara yang mungkin Anda gunakan untuk membuat hash password secara manual untuk pengguna awal atau admin sebelum mengintegrasikan fungsi pendaftaran penuh.
Langkah Selanjutnya
Setelah struktur ini siap, Anda bisa mulai mengisi setiap file dengan kode yang sesuai. Misalnya:

src/config/database.js: Menyiapkan koneksi ke database.
src/models/User.js: Menulis kueri SQL untuk operasi CRUD pada tabel pengguna.
src/middleware/authMiddleware.js: Mengimplementasikan logika untuk memverifikasi token JWT dan otorisasi peran.
src/controllers/authController.js: Menangani logika login dan pendaftaran, serta pembuatan token JWT.
src/routes/authRoutes.js: Mendefinisikan rute seperti /login, /register.
src/app.js: Menghubungkan semuanya dan memulai server Express.

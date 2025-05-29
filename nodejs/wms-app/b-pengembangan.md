# Capaian Pengembangan WMS App

Dokumen ini mencatat progres signifikan, implementasi fitur, dan penyelesaian masalah teknis dalam pengembangan aplikasi Warehouse Management System (WMS) berbasis Node.js, Express, dan MySQL. Ini merupakan lanjutan dan pendalaman dari dokumen panduan awal "Membangun Modul Autentikasi Pengguna (User Login)".

## Daftar Isi

1.Gambaran Umum Proyek & Capaian

2.Fondasi Teknis & Lingkungan Pengembangan

3.Struktur Folder Proyek

4.Fitur Utama yang Telah Diimplementasikan

  4.1. Modul Autentikasi Pengguna
  
  4.2. Manajemen Pengguna (Role Admin)
  
  4.3. Role-Based Access Control (RBAC) & Dashboard Spesifik Role

5.Pembaruan & Implementasi Kode Signifikan

  5.1. src/models/User.js (Pembaruan Model Pengguna)
  
  5.2. src/controllers/userController.js (Modul Manajemen Pengguna)
  
  5.3. src/routes/adminRoutes.js (Rute Administrasi)
  
  5.4. src/app.js (Konfigurasi Aplikasi Utama)
  
  5.5. views/ (Tampilan EJS)

6.Tantangan Teknis Kritis & Solusinya

7.Instruksi Menjalankan Aplikasi

# 1. Gambaran Umum Proyek & Capaian

Proyek WMS App ini bertujuan untuk membangun sistem manajemen gudang dasar. Fase pengembangan saat ini berfokus pada pembangunan fondasi autentikasi pengguna yang kuat dan modul administrasi pengguna yang komprehensif. Aplikasi ini memungkinkan berbagai role pengguna untuk login dan mengakses dashboard atau fungsionalitas yang sesuai dengan izin mereka.

Capaian Utama Hingga Saat Ini:

  -Sistem autentikasi (login, logout) berfungsi penuh, menggunakan JWT untuk manajemen sesi dan Bcrypt untuk hashing password yang aman.

  -Pengguna dapat berhasil login dan diarahkan ke dashboard yang disesuaikan berdasarkan role mereka (admin, ppic, packing, mixing, milling, qc, gudang).

  -Modul Manajemen Pengguna untuk role admin telah diimplementasikan sepenuhnya, mencakup fungsionalitas CRUD (Create, Read, Update, Delete) pengguna lain.

  -Penerapan Role-Based Access Control (RBAC) dasar untuk melindungi route dan dashboard tertentu.

  -Penanganan error dan penyediaan feedback pesan yang informatif di frontend (melalui EJS) setelah operasi pengguna.

  -Struktur proyek dan file-file view EJS telah tertata rapi dan siap untuk pengembangan modul selanjutnya.

# 2. Fondasi Teknis & Lingkungan Pengembangan

  -Runtime: Node.js
 
  -Web Framework: Express.js
 
  -Templating Engine: EJS (Embedded JavaScript)
 
  -Database: MySQL / MariaDB (melalui mysql2/promise)
 
  -Hashing Password: bcryptjs
 
  -Autentikasi Sesi: JSON Web Tokens (JWT) dengan jsonwebtoken
 
  -Manajemen Environment Variables: dotenv
 
  -Parsing Cookies: cookie-parser
 
  -Pengembangan: nodemon (direkomendasikan untuk auto-restart server)

# 3. Struktur Folder Proyek

``
wms-app/
├── node_modules/           # Dependencies
├── src/
│   ├── config/             # Konfigurasi database
│   │   └── database.js
│   ├── controllers/        # Logika bisnis untuk setiap fungsionalitas
│   │   ├── authController.js
│   │   └── userController.js  # Kontroler untuk manajemen pengguna (admin)
│   ├── models/             # Interaksi dengan database (SQL queries)
│   │   └── User.js            # Model untuk tabel `users`
│   ├── routes/             # Definisi API endpoints
│   │   ├── authRoutes.js
│   │   └── adminRoutes.js     # Rute khusus untuk fungsi admin
│   ├── middleware/         # Middleware kustom (autentikasi, otorisasi)
│   │   └── authMiddleware.js
│   └── app.js              # File utama aplikasi Express
├── views/                  # Folder untuk template EJS
│   ├── auth/
│   │   └── login.ejs          # Form Login
│   ├── admin/
│   │   └── users/             # Tampilan manajemen pengguna admin
│   │       ├── list.ejs       # Daftar pengguna
│   │       ├── create.ejs     # Form tambah pengguna baru
│   │       └── edit.ejs       # Form edit detail pengguna
│   │   └── change-password.ejs  # Form ganti password untuk admin
│   ├── dashboard/             # Dashboard spesifik untuk setiap role
│   │   ├── admin.ejs
│   │   ├── ppic.ejs
│   │   ├── packing.ejs
│   │   ├── mixing.ejs
│   │   ├── milling.ejs
│   │   ├── qc.ejs
│   │   └── gudang.ejs
│   └── error.ejs             # Halaman error umum
├── .env                    # Environment variables (private)
├── .env.example            # Contoh template .env
├── .gitignore
├── package.json
├── package-lock.json
└── hash_password.js        # Utilitas sementara untuk generate hash password
``

# 4. Fitur Utama yang Telah Diimplementasikan
 
  4.1. Modul Autentikasi Pengguna
  
    Login: Pengguna dapat login dengan username dan password. Validasi input dan password hashing dilakukan di sisi backend.

    Logout: Menghapus sesi pengguna dengan membersihkan HTTP-only cookie token.

    JWT Sesi: Penggunaan JSON Web Tokens yang disimpan di HTTP-only cookie untuk manajemen sesi yang aman.

    bcryptjs: Penerapan hashing password dengan bcryptjs untuk keamanan penyimpanan password di database.

  4.2. Manajemen Pengguna (Role Admin)

    Daftar Pengguna (/admin/users): Menampilkan tabel interaktif semua pengguna yang terdaftar, termasuk detail penting dan status aktif.

    Tambah Pengguna Baru (/admin/users/create): Menyediakan formulir untuk admin dalam membuat akun pengguna baru dengan berbagai role.

    Edit Detail Pengguna (/admin/users/:id/edit): Memungkinkan admin untuk memodifikasi informasi pengguna yang sudah ada (kecuali password).

    Hapus Pengguna (/admin/users/:id/delete): Fungsionalitas untuk menghapus pengguna, dengan middleware dan validasi untuk mencegah admin menghapus akunnya sendiri.

    Ganti Password Admin (/admin/change-password): Formulir khusus bagi admin untuk mengubah password akun mereka sendiri secara aman, dengan verifikasi password lama.

  4.3. Role-Based Access Control (RBAC) & Dashboard Spesifik Role

    Setiap route dashboard (/dashboard/:role) dilindungi oleh middleware authenticateToken dan authorizeRoles, memastikan hanya pengguna dengan role yang diizinkan yang dapat mengaksesnya.

    Redirect Otomatis: Setelah login sukses, pengguna akan otomatis dialihkan ke dashboard yang sesuai dengan role mereka (misalnya, admin ke /dashboard/admin, ppic ke /dashboard/ppic, dst.).

# 5. Pembaruan & Implementasi Kode Signifikan
 
  5.1. src/models/User.js (Pembaruan Model Pengguna)
  
    Menambahkan method statis untuk interaksi database:
    
      static async getAll(): Mengambil semua data pengguna.
      
      static async create(userData): Menambahkan record pengguna baru.
      
      static async update(id, updateData): Memperbarui record pengguna berdasarkan ID.
      
      static async delete(id): Menghapus record pengguna berdasarkan ID.
      
      static async updatePassword(id, newPasswordHash): Memperbarui hash password pengguna tertentu.
      
      static async findByEmail(email): Mendapatkan pengguna berdasarkan alamat email.
  
  5.2. src/controllers/userController.js (Modul Manajemen Pengguna)

    File ini baru diimplementasikan untuk mengelola semua logika bisnis terkait CRUD pengguna.

    Mengandung fungsi-fungsi seperti listUsers, showCreateUserForm, createUser, showEditUserForm, updateUser, deleteUser, showChangePasswordForm, dan changePassword.

    Mengintegrasikan penanganan redirect dengan query parameter (?message=... atau ?error=...) untuk menampilkan feedback di frontend.

  5.3. src/routes/adminRoutes.js (Rute Administrasi)

    Mendefinisikan semua endpoint HTTP untuk operasi manajemen pengguna (GET, POST).

    Setiap route dilindungi oleh authenticateToken dan authorizeRoles('admin') untuk memastikan hanya admin yang dapat mengakses fungsionalitas ini.

  5.4. src/app.js (Konfigurasi Aplikasi Utama)

    Integrasi adminRoutes dengan prefix /admin.
  
    Penyempurnaan logika redirect di root route (/) dan semua dashboard routes (/dashboard/:role) untuk memastikan objek pengguna yang terautentikasi (req.user) selalu diteruskan ke view sebagai variabel user.
    
    Penambahan console.log yang informatif untuk URL-URL penting saat server dimulai.
  
  5.5. views/ (Tampilan EJS)
    
    views/admin/users/: Folder baru berisi list.ejs, create.ejs, edit.ejs, change-password.ejs untuk antarmuka manajemen pengguna.

    views/dashboard/: Semua file dashboard untuk setiap role (admin.ejs, ppic.ejs, packing.ejs, mixing.ejs, milling.ejs, qc.ejs, gudang.ejs) telah dibuat dan disesuaikan.

    Koreksi Variabel: Semua file EJS di views/dashboard/ dan views/admin/users/ yang menampilkan data pengguna sekarang secara konsisten menggunakan variabel user (bukan currentUser) untuk mengakses detail pengguna yang terautentikasi (contoh: <%= user.full_name %>).

# 6. Tantangan Teknis Kritis & Solusinya

    Selama fase pengembangan ini, beberapa error dan hambatan teknis berhasil diidentifikasi dan diatasi:

    SyntaxError: Identifier 'pool' has already been declared:

    Penyebab: Deklarasi ulang variabel pool di lingkup yang sama atau penanganan module exports/imports yang tidak tepat.

    Solusi: Memastikan pool hanya dideklarasikan sekali di src/config/database.js dan diekspor dengan benar, kemudian di-require tanpa deklarasi ulang di modul lain.

    TypeError: User.findByUsername is not a function (dan sejenisnya untuk method model lain):

    Penyebab: Method yang dipanggil tidak didefinisikan atau tidak diekspor/di-require dengan benar di objek User.

    Solusi: Mengimplementasikan semua method CRUD dan pencarian (findByUsername, findById, getAll, create, update, delete, updatePassword, findByEmail) sebagai method statis di src/models/User.js dan memastikan module.exports = User; di bagian akhir.

    Error: Failed to lookup view "admin/users/create" (dan serupa untuk view admin/dashboard lainnya):

    Penyebab: Jalur yang diberikan ke res.render() tidak cocok dengan lokasi fisik file EJS di folder views/.
    
    Solusi: Mengoreksi jalur rendering di controller agar sesuai dengan struktur folder views/admin/users/ yang baru, dan memastikan semua file EJS yang diperlukan (misal: list.ejs, create.ejs, edit.ejs, serta semua dashboard EJS) benar-benar ada di path yang ditentukan.

    ReferenceError: message is not defined (di list.ejs):

    Penyebab: Variabel message (atau error) diakses di template EJS tanpa variabel tersebut diteruskan dari controller.

    Solusi: Memodifikasi exports.listUsers di src/controllers/userController.js untuk secara eksplisit meneruskan message dan error (bisa null secara default) ke res.render(). Ini juga melibatkan pengiriman pesan sukses/error melalui query parameter pada redirect (misal: res.redirect('/admin/users?message=Pengguna berhasil dibuat!');).
    
    ReferenceError: currentUser is not defined (di dashboard EJS files seperti ppic.ejs):

    Penyebab: Ketidakcocokan nama variabel. Controller (app.js atau *authController.js) mengirim data pengguna dengan kunciuser({ user: req.user }), tetapi *template* EJS mencoba mengaksesnya dengan kuncicurrentUser`.

    Solusi: Mengubah semua referensi currentUser menjadi user di semua file EJS dashboard (admin.ejs, ppic.ejs, dst.) agar konsisten dengan cara data dikirim dari backend.

# 7. Instruksi Menjalankan Aplikasi

  Asumsikan Anda telah melakukan persiapan awal proyek sesuai dokumen "Membangun Modul Autentikasi Pengguna (User Login)".

    Pastikan MySQL Server berjalan.

    Pastikan Anda memiliki database wms_db dan tabel users dengan setidaknya satu pengguna admin (dengan password_hash yang telah di-generate dan dimasukkan).

    Instal semua dependensi Node.js:
    
    Bash

    npm install

    Siapkan file .env di root proyek Anda (wms-app/) dengan konfigurasi database (DB_HOST, DB_USER, DB_PASSWORD, DB_NAME) dan secret key JWT (JWT_SECRET). Anda bisa merujuk ke a.User Login.md atau env.example jika tersedia.

    Jalankan aplikasi Node.js:
    
    Bash

    node src/app.js

    (Direkomendasikan menggunakan nodemon untuk auto-restart saat perubahan kode, jika sudah terinstal: nodemon src/app.js)

    Akses aplikasi di browser Anda:
    
      Halaman Login: http://localhost:3000/login
      
      Dashboard Admin: http://localhost:3000/dashboard/admin (setelah login sebagai admin)
      
      Manajemen Pengguna (Admin):

      Buat Pengguna Baru: http://localhost:3000/admin/users/create

      Lihat Daftar Pengguna: http://localhost:3000/admin/users

      Ubah Password Admin: http://localhost:3000/admin/change-password






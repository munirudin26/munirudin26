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

```
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
```

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


# Dokumentasi code
`wms-app/hash_password.js`
```
// hash_password.js
const bcrypt = require('bcryptjs');

async function hashPassword() {
    const passwordToHash = 'admin123'; // Password yang ingin di-hash
    const saltRounds = 10; // Kekuatan hashing (10 adalah nilai yang baik)

    try {
        const hashedPassword = await bcrypt.hash(passwordToHash, saltRounds);
        console.log('Hashed Password untuk "admin123":', hashedPassword);
    } catch (error) {
        console.error('Error hashing password:', error);
    }
}
```

`wms-app/package.json`

`wms-app/package-lock.json`

`wms-app/node_modules/`

`wms/src/app.js`
```
// src/app.js
const express = require('express');
const path = require('path');
const cookieParser = require('cookie-parser');
const { connectDB } = require('./config/database');
const authRoutes = require('./routes/authRoutes');
const adminRoutes = require('./routes/adminRoutes');
const { authenticateToken, authorizeRoles } = require('./middleware/authMiddleware');
require('dotenv').config();

const app = express();
const PORT = process.env.PORT || 3000;

// Menghubungkan ke database saat aplikasi dimulai
connectDB();

// Middleware dasar Express
app.use(express.urlencoded({ extended: true })); // Untuk parsing data dari form HTML
app.use(express.json()); // Untuk parsing body JSON
app.use(cookieParser()); // Untuk mem-parsing dan membaca cookies

// Mengatur EJS sebagai templating engine
app.set('view engine', 'ejs');
app.set('views', path.join(__dirname, '../views')); // Menentukan lokasi folder views

// Mengatur folder 'public' untuk file statis (CSS, JS frontend, gambar)
app.use(express.static(path.join(__dirname, '../public')));

// Menggunakan route autentikasi (publik, tidak memerlukan login)
app.use('/', authRoutes);
//app.use('/', adminRoutes); // Tambahkan ini
app.use('/admin', adminRoutes); // <- Saran saya menggunakan prefix
// Route utama '/' akan dicek autentikasi sebelum diarahkan ke dashboard
app.get('/', authenticateToken, (req, res) => {
    // Redirect ke dashboard yang sesuai dengan role user setelah login
    switch (req.user.role) {
        case 'admin':
            return res.redirect('/dashboard/admin');
        case 'ppic':
            return res.redirect('/dashboard/ppic');
        case 'packing':
            return res.redirect('/dashboard/packing');
        case 'mixing':
            return res.redirect('/dashboard/mixing');
        case 'milling':
            return res.redirect('/dashboard/milling');
        case 'qc':
            return res.redirect('/dashboard/qc');
        case 'gudang':
            return res.redirect('/dashboard/gudang');
        default:
            // Jika role tidak dikenali, bisa diarahkan ke halaman default atau error
            return res.render('dashboard', { user: req.user, message: 'Selamat datang di WMS!' 
});
    }
});

// Contoh Route Dashboard yang Dilindungi per Role
// Setiap role memiliki dashboardnya sendiri yang hanya bisa diakses oleh role tersebut (dan ad
min)
app.get('/dashboard/admin', authenticateToken, authorizeRoles('admin'), (req, res) => {
    res.render('dashboard/admin', { user: req.user });
});

app.get('/dashboard/ppic', authenticateToken, authorizeRoles('admin', 'ppic'), (req, res) => {
    res.render('dashboard/ppic', { user: req.user });
});

app.get('/dashboard/packing', authenticateToken, authorizeRoles('admin', 'packing'), (req, res)
 => {
    res.render('dashboard/packing', { user: req.user });
});

app.get('/dashboard/mixing', authenticateToken, authorizeRoles('admin', 'mixing'), (req, res) =
> {
    res.render('dashboard/mixing', { user: req.user });
});

app.get('/dashboard/milling', authenticateToken, authorizeRoles('admin', 'milling'), (req, res)
 => {
    res.render('dashboard/milling', { user: req.user });
});

app.get('/dashboard/qc', authenticateToken, authorizeRoles('admin', 'qc'), (req, res) => {
    res.render('dashboard/qc', { user: req.user });
});

app.get('/dashboard/gudang', authenticateToken, authorizeRoles('admin', 'gudang'), (req, res) =
> {
    res.render('dashboard/gudang', { user: req.user });
});

// Jalankan server
app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
    // --- TAMBAHKAN BARIS INI UNTUK MENAMPILKAN URL PENTING ---
    console.log('\n--- URL Penting (Akses setelah Login sebagai Admin) ---');
    console.log(`1. Buat Pengguna Baru: http://localhost:${PORT}/admin/users/create`);
    console.log(`2. Lihat Daftar Pengguna: http://localhost:${PORT}/admin/users`);
    console.log(`3. Ubah Password Admin: http://localhost:${PORT}/admin/change-password`);
    console.log('----------------------------------------------------');
    // --- AKHIR PENAMBAHAN ---
});
```

`wms-app/src/config/database.js`
```
// src/config/database.js
const mysql = require('mysql2/promise');
require('dotenv').config(); // Memuat variabel lingkungan dari .env

const pool = mysql.createPool({
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASSWORD,
    database: process.env.DB_NAME,
    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0
});

async function connectDB() {
    try {
        const connection = await pool.getConnection();
        console.log('Connected to MySQL database!');
        connection.release(); // Mengembalikan koneksi ke pool

        // Opsional: Buat database dan tabel users jika belum ada
        // Biasanya ini hanya untuk pengembangan awal, di produksi lebih baik melalui migrasi
        await pool.query(`CREATE DATABASE IF NOT EXISTS ${process.env.DB_NAME}`);
        console.log(`Database '${process.env.DB_NAME}' checked/created.`);
        await createUsersTable(); // Panggil fungsi untuk membuat tabel users

    } catch (err) {
        console.error('Error connecting to database:', err.message);
        process.exit(1); // Keluar aplikasi jika koneksi database gagal
    }
}

async function createUsersTable() {
    const createUserTableQuery = `
    CREATE TABLE IF NOT EXISTS users (
        id INT AUTO_INCREMENT PRIMARY KEY,
        username VARCHAR(50) UNIQUE NOT NULL,
        password_hash VARCHAR(255) NOT NULL,
        full_name VARCHAR(100) NOT NULL,
        email VARCHAR(100) UNIQUE,
        role ENUM('admin', 'ppic', 'packing', 'mixing', 'milling', 'qc', 'gudang') NOT NULL,
        is_active BOOLEAN DEFAULT TRUE,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
    );
    `;

    try {
        await pool.query(createUserTableQuery);
        console.log('Table `users` checked/created by app.');
    } catch (err) {
        console.error('Error creating `users` table:', err.message);
    }
}

module.exports = {
    pool,
    connectDB
};
```

`wms-app/src/controllers/authController.js`
```
const crypto = require('crypto');
const nodemailer = require('nodemailer');
// src/controllers/authController.js
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const User = require('../models/User');
require('dotenv').config(); // Memuat variabel lingkungan

exports.showLoginForm = (req, res) => {
    // Merender halaman login EJS, mengirimkan variabel error (jika ada)
    res.render('auth/login', { error: null });
};

const transporter = nodemailer.createTransport({
    host: process.env.EMAIL_HOST,
    port: process.env.EMAIL_PORT,
    secure: false,
    auth: {
        user: process.env.EMAIL_USER,
        pass: process.env.EMAIL_PASS
    }
});

exports.login = async (req, res) => {
    const { username, password } = req.body; // Mengambil username dan password dari body request

    // Validasi input dasar
    if (!username || !password) {
        return res.render('auth/login', { error: 'Username dan password harus diisi.' });
    }

    try {
        // Mencari user di database berdasarkan username
        const user = await User.findByUsername(username);

        // Jika user tidak ditemukan
        if (!user) {
            return res.render('auth/login', { error: 'Username atau password salah.' });
        }

        // Membandingkan password yang diinput dengan hash di database
        const isMatch = await bcrypt.compare(password, user.password_hash);

        // Jika password tidak cocok
        if (!isMatch) {
            return res.render('auth/login', { error: 'Username atau password salah.' });
        }

        // Generate JWT Token
        const token = jwt.sign(
            { id: user.id, role: user.role, fullName: user.full_name }, // Payload token
            process.env.JWT_SECRET, // Secret key dari .env
            { expiresIn: '1h' } // Token berlaku selama 1 jam
        );

        // Set token sebagai HTTP-only cookie (lebih aman dari local storage)
        res.cookie('token', token, {
            httpOnly: true, // Tidak bisa diakses oleh JavaScript di sisi klien
            secure: process.env.NODE_ENV === 'production', // Hanya kirim via HTTPS di mode produksi
            maxAge: 3600000 // 1 jam dalam milidetik
        });
        // Redirect pengguna ke dashboard yang sesuai dengan rolenya
        switch (user.role) {
            case 'admin':
                return res.redirect('/dashboard/admin');
            case 'ppic':
                return res.redirect('/dashboard/ppic');
            case 'packing':
                return res.redirect('/dashboard/packing');
            case 'mixing':
                return res.redirect('/dashboard/mixing');
            case 'milling':
                return res.redirect('/dashboard/milling');
            case 'qc':
                return res.redirect('/dashboard/qc');
            case 'gudang':
                return res.redirect('/dashboard/gudang');
            default:
                return res.redirect('/'); // Default redirect jika role tidak dikenali
        }

    } catch (err) {
        console.error('Login error:', err);
        res.render('auth/login', { error: 'Terjadi kesalahan server.' });
    }
};

exports.logout = (req, res) => {
    // Menghapus cookie token dan mengarahkan kembali ke halaman login
    res.clearCookie('token');
    res.redirect('/login');
};

exports.showRegisterForm = (req, res) => {
    res.render('auth/register', { error: null });
};

exports.register = async (req, res) => {
    const { username, password, confirm_password, full_name, email, role } = req.body;

    // Validasi
    if (password !== confirm_password) {
        return res.render('auth/register', { error: 'Password dan konfirmasi password tidak cocok.' });
    }

    try {
        // Cek apakah username atau email sudah digunakan
        const existingUser = await User.findByUsername(username) || await User.findByEmail(email);
        if (existingUser) {
            return res.render('auth/register', { 
                error: 'Username atau email sudah digunakan.' 
            });
        }
        // Hash password
        const saltRounds = 10;
        const password_hash = await bcrypt.hash(password, saltRounds);

        // Buat user baru (default role 'user' jika tidak ada role yang ditentukan)
        const newUser = {
            username,
            password_hash,
            full_name,
            email,
            role: role || 'user'
        };

        const userId = await User.create(newUser);

        // Generate verification token
        const verificationToken = crypto.randomBytes(32).toString('hex');
        await User.setVerificationToken(userId, verificationToken);

        // Kirim email verifikasi
        const verificationUrl = `${req.protocol}://${req.get('host')}/verify-email?token=${verificationToken}`;
        
        await transporter.sendMail({
            from: process.env.EMAIL_FROM,
            to: email,
            subject: 'Verifikasi Email Anda',
            html: `Silakan klik link berikut untuk verifikasi email: <a href="${verificationUrl}">${verificationUrl}</a>`
        });

        res.render('auth/register-success', { email });

    } catch (err) {
        console.error('Registration error:', err);
        res.render('auth/register', { error: 'Terjadi kesalahan saat registrasi.' });
    }
};

exports.showForgotPasswordForm = (req, res) => {
    res.render('auth/forgot-password', { error: null, success: null });
};

exports.forgotPassword = async (req, res) => {
    const { email } = req.body;

    try {
        const user = await User.findByEmail(email);
        if (!user) {
            return res.render('auth/forgot-password', { 
                error: 'Email tidak ditemukan.', 
                success: null 
            });
        }
        // Generate reset token (expires in 1 hour)
        const resetToken = crypto.randomBytes(32).toString('hex');
        const resetTokenExpires = new Date(Date.now() + 3600000); // 1 jam

        await User.setResetToken(user.id, resetToken, resetTokenExpires);

        // Kirim email reset password
        const resetUrl = `${req.protocol}://${req.get('host')}/reset-password?token=${resetToken}`;
        
        await transporter.sendMail({
            from: process.env.EMAIL_FROM,
            to: user.email,
            subject: 'Reset Password Anda',
            html: `Silakan klik link berikut untuk reset password: <a href="${resetUrl}">${resetUrl}</a>`
        });

        res.render('auth/forgot-password', { 
            error: null, 
            success: 'Link reset password telah dikirim ke email Anda.' 
        });

    } catch (err) {
        console.error('Forgot password error:', err);
        res.render('auth/forgot-password', { 
            error: 'Terjadi kesalahan saat memproses permintaan.', 
            success: null 
        });
    }
};

exports.showResetPasswordForm = async (req, res) => {
    const { token } = req.query;

    try {
        const user = await User.findByResetToken(token);
        if (!user) {
            return res.render('auth/reset-password', { 
                error: 'Token reset password tidak valid atau sudah kadaluarsa.', 
                token: null 
            });
        }

        res.render('auth/reset-password', { 
            error: null, 
            token 
        });

    } catch (err) {
        console.error('Reset password error:', err);
        res.render('auth/reset-password', { 
            error: 'Terjadi kesalahan saat memproses permintaan.', 
            token: null 
        });
    }
};
exports.resetPassword = async (req, res) => {
    const { token, password, confirm_password } = req.body;

    if (password !== confirm_password) {
        return res.render('auth/reset-password', { 
            error: 'Password dan konfirmasi password tidak cocok.', 
            token 
        });
    }

    try {
        const user = await User.findByResetToken(token);
        if (!user) {
            return res.render('auth/reset-password', { 
                error: 'Token reset password tidak valid atau sudah kadaluarsa.', 
                token: null 
            });
        }

        // Hash password baru
        const saltRounds = 10;
        const password_hash = await bcrypt.hash(password, saltRounds);

        // Update password dan clear reset token
        await User.updatePassword(user.id, password_hash);

        res.render('auth/reset-password-success');

    } catch (err) {
        console.error('Reset password error:', err);
        res.render('auth/reset-password', { 
            error: 'Terjadi kesalahan saat reset password.', 
            token 
        });
    }
};

exports.verifyEmail = async (req, res) => {
    const { token } = req.query;

    try {
        const isVerified = await User.verifyEmail(token);
        if (isVerified) {
            res.render('auth/verify-email-success');
        } else {
            res.render('auth/verify-email-error');
        }
    } catch (err) {
        console.error('Email verification error:', err);
        res.render('auth/verify-email-error');
    }
};
```

`wms-app/src/controllers/userController.js`
```
const User = require('../models/User');
const bcrypt = require('bcryptjs');

exports.listUsers = async (req, res) => {
    try {
        const users = await User.getAll();
        // Ambil 'message' dan 'error' dari query parameters, atau set null/undefined secara default
        // Ini memungkinkan pesan sukses/error dari redirect sebelumnya (misal: setelah createUser) untuk ditampilkan
        const message = req.query.message || null;
        const error = req.query.error || null;

        res.render('admin/users/list', {
            users,
            currentUser: req.user,
            message: message, // <-- Tambahkan ini
            error: error      // <-- Tambahkan ini
        });
    } catch (err) {
        console.error('Error fetching users:', err);
        res.render('error', { message: 'Gagal mengambil data pengguna.' });
    }
};

exports.showCreateUserForm = (req, res) => {
    res.render('admin/users/create', {
        error: null,
        formData: null
    });
};

exports.createUser = async (req, res) => {
    const { username, password, full_name, email, role } = req.body;

    try {
        // Validasi
        const existingUser = await User.findByUsername(username) || await User.findByEmail(email);
        if (existingUser) {
            return res.render('admin/users/create', {
                error: 'Username atau email sudah digunakan.',
                formData: req.body
            });
        }
        // Hash password
        const saltRounds = 10;
        const password_hash = await bcrypt.hash(password, saltRounds);

        // Buat user baru
        const newUser = {
            username,
            password_hash,
            full_name,
            email,
            role,
            is_active: true
        };

        await User.create(newUser);

        // Redirect dengan pesan sukses
        res.redirect('/admin/users?message=Pengguna berhasil dibuat!'); // <-- Modifikasi redirect di sini

    } catch (err) {
        console.error('Create user error:', err);
        res.render('admin/users/create', {
            error: 'Terjadi kesalahan saat membuat pengguna.',
            formData: req.body
        });
    }
};

exports.showEditUserForm = async (req, res) => {
    const { id } = req.params;

    try {
        const user = await User.findById(id);
        if (!user) {
            return res.render('error', { message: 'Pengguna tidak ditemukan.' });
        }
        res.render('admin/users/edit', {
            user,
            error: null
        });

    } catch (err) {
        console.error('Error fetching user:', err);
        res.render('error', { message: 'Gagal mengambil data pengguna.' });
    }
};
exports.updateUser = async (req, res) => {
    const { id } = req.params;
    const { username, full_name, email, role, is_active } = req.body;

    try {
        const updateData = {
            username,
            full_name,
            email,
            role,
            is_active: is_active === 'on'
        };

        await User.update(id, updateData);

        // Redirect dengan pesan sukses
        res.redirect('/admin/users?message=Pengguna berhasil diperbarui!'); // <-- Modifikasi redirect di sini
    } catch (err) {
        console.error('Update user error:', err);
        res.render('admin/users/edit', {
            user: req.body,
            error: 'Terjadi kesalahan saat memperbarui pengguna.'
        });
    }
};

exports.deleteUser = async (req, res) => {
    const { id } = req.params;

    try {
        // Jangan izinkan menghapus diri sendiri
        if (parseInt(id) === req.user.id) {
            return res.render('error', {
                message: 'Anda tidak dapat menghapus akun sendiri.'
            });
        }

        await User.delete(id);
        // Redirect dengan pesan sukses
        res.redirect('/admin/users?message=Pengguna berhasil dihapus!'); // <-- Modifikasi redirect di sini

    } catch (err) {
        console.error('Delete user error:', err);
        res.render('error', { message: 'Gagal menghapus pengguna.' });
    }
};
exports.showChangePasswordForm = async (req, res) => {
    res.render('admin/users/change-password', {
        error: null,
        success: null
    });
};

exports.changePassword = async (req, res) => {
    const { current_password, new_password, confirm_password } = req.body;

    if (new_password !== confirm_password) {
        return res.render('admin/users/change-password', {
            error: 'Password baru dan konfirmasi password tidak cocok.',
            success: null
        });
    }

    try {
        // Dapatkan user saat ini
        const user = await User.findById(req.user.id);

        // Verifikasi password saat ini
        const isMatch = await bcrypt.compare(current_password, user.password_hash);
        if (!isMatch) {
            return res.render('admin/users/change-password', {
                error: 'Password saat ini salah.',
                success: null
            });
        }

        // Hash password baru
        const saltRounds = 10;
        const password_hash = await bcrypt.hash(new_password, saltRounds);

        // Update password
        await User.updatePassword(user.id, password_hash);

        res.render('admin/users/change-password', {
            error: null,
            success: 'Password berhasil diubah.'
        });
    } catch (err) {
        console.error('Change password error:', err);
        res.render('admin/users/change-password', {
            error: 'Terjadi kesalahan saat mengubah password.',
            success: null
        });
    }
};
```

`wms-app/src/middleware/authMiddleware.js`
```
// src/middleware/authMiddleware.js
const jwt = require('jsonwebtoken');
require('dotenv').config(); // Memuat variabel lingkungan

exports.authenticateToken = (req, res, next) => {
    const token = req.cookies.token; // Mengambil token dari cookies

    // Jika tidak ada token, user belum login atau sesi sudah habis
    if (!token) {
        return res.redirect('/login'); // Arahkan ke halaman login
    }

    // Memverifikasi token
    jwt.verify(token, process.env.JWT_SECRET, (err, user) => {
        if (err) {
            // Token tidak valid atau kadaluarsa
            res.clearCookie('token'); // Hapus cookie yang tidak valid
            return res.redirect('/login'); // Arahkan kembali ke login
        }
        req.user = user; // Menambahkan payload user (id, role, fullName) ke objek request
        next(); // Lanjutkan ke middleware/route berikutnya
    });
};

exports.authorizeRoles = (...roles) => {
    return (req, res, next) => {
        // Memastikan user sudah terautentikasi dan rolenya sesuai
        if (!req.user || !roles.includes(req.user.role)) {
            // Jika user tidak ada atau rolenya tidak diizinkan, kembalikan status 403 (Forbidden)
            return res.status(403).render('error', { message: 'Akses Ditolak: Anda tidak memiliki izin untuk halaman ini.' });
        }
        next(); // Lanjutkan
    };
};
```

`wms-app/src/models/User.js`
```
// src/models/User.js
const { pool } = require('../config/database'); // HANYA ADA SATU DEKLARASI pool DI SINI

class User {
    // Pindahkan metode findByUsername dan findById ke sini
    static async findByUsername(username) {
        const [rows] = await pool.query('SELECT * FROM users WHERE username = ?', [username]);
        return rows[0]; // Mengembalikan baris pertama jika ditemukan
    }

    static async findById(id) {
        const [rows] = await pool.query('SELECT * FROM users WHERE id = ?', [id]);
        return rows[0];
    }

    // --- Semua metode lainnya yang sudah Anda buat ---

    static async create(userData) {
        const { username, password_hash, full_name, email, role } = userData;
        const [result] = await pool.query(
            'INSERT INTO users (username, password_hash, full_name, email, role) VALUES (?, ?, ?, ?, ?)',
            [username, password_hash, full_name, email, role]
        );
        return result.insertId;
    }

    static async update(id, userData) {
        const { username, full_name, email, role, is_active } = userData;
        await pool.query(
            'UPDATE users SET username = ?, full_name = ?, email = ?, role = ?, is_active = ?, updated_at = CURRENT_TIMESTAMP WHERE i
d = ?',
            [username, full_name, email, role, is_active, id]
        );
    }

    static async delete(id) {
        await pool.query('DELETE FROM users WHERE id = ?', [id]);
    }

    static async getAll() {
        const [rows] = await pool.query('SELECT id, username, full_name, email, role, is_active FROM users');
        return rows;
    }

    static async setResetToken(id, token, expires) {
        await pool.query(
            'UPDATE users SET reset_token = ?, reset_token_expires = ? WHERE id = ?',
            [token, expires, id]
        );
    }
    static async findByResetToken(token) {
        const [rows] = await pool.query(
            'SELECT * FROM users WHERE reset_token = ? AND reset_token_expires > NOW()',
            [token]
        );
        return rows[0];
    }

    static async updatePassword(id, password_hash) {
        await pool.query(
            'UPDATE users SET password_hash = ?, reset_token = NULL, reset_token_expires = NULL WHERE id = ?',
            [password_hash, id]
        );
    }

    static async findByEmail(email) {
        const [rows] = await pool.query('SELECT * FROM users WHERE email = ?', [email]);
        return rows[0];
    }

    static async setVerificationToken(id, token) {
        await pool.query(
            'UPDATE users SET verification_token = ? WHERE id = ?',
            [token, id]
        );
    }

    static async verifyEmail(token) {
        const [result] = await pool.query(
            'UPDATE users SET email_verified = TRUE, verification_token = NULL WHERE verification_token = ?',
            [token]
        );
        return result.affectedRows > 0;
    }
}

module.exports = User; // HANYA ADA SATU exports DI SINI
```

`wms-app/src/routes/adminRoutes.js`
```
//code 2
const express = require('express');
const router = express.Router();
const userController = require('../controllers/userController');
const { authenticateToken, authorizeRoles } = require('../middleware/authMiddleware');

// Admin-only middleware
router.use(authenticateToken);
router.use(authorizeRoles('admin'));

// User management routes (tanpa /admin/ di depannya)
router.get('/users', userController.listUsers); // Akan diakses sebagai /admin/users
router.get('/users/create', userController.showCreateUserForm); // Akan diakses sebagai /admin/users/create
router.post('/users', userController.createUser);
router.get('/users/:id/edit', userController.showEditUserForm);
router.post('/users/:id', userController.updateUser);
router.post('/users/:id/delete', userController.deleteUser);

// Change password (tanpa /admin/ di depannya)
router.get('/change-password', userController.showChangePasswordForm); // Akan diakses sebagai /admin/change-password
router.post('/change-password', userController.changePassword);

module.exports = router;
```

`wms-app/src/routes/authRoutes.js`
```
const express = require('express');
const router = express.Router();
const authController = require('../controllers/authController');

// Authentication routes
router.get('/login', authController.showLoginForm);
router.post('/login', authController.login);
router.get('/logout', authController.logout);

// Registration routes
router.get('/register', authController.showRegisterForm);
router.post('/register', authController.register);

// Password reset routes
router.get('/forgot-password', authController.showForgotPasswordForm);
router.post('/forgot-password', authController.forgotPassword);
router.get('/reset-password', authController.showResetPasswordForm);
router.post('/reset-password', authController.resetPassword);

// Email verification
router.get('/verify-email', authController.verifyEmail);

module.exports = router;
```

`wms-app/views/error.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Akses Ditolak</title>
</head>
<body>
    <h1>Akses Ditolak</h1>
    <p><%= message %></p>
    <a href="/">Kembali ke Home</a>
</body>
</html>
```

`wms-app/views/admin/users/change-password.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ubah Password</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: flex-start;
            min-height: 100vh;
        }
        .container {
            background-color: #fff;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 450px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 20px;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #555;
            font-weight: bold;
        }
        input[type="password"] {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
            margin-top: 5px;
        }
        button {
            background-color: #ffc107; /* Yellow for change password */
            color: #333;
            padding: 12px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            width: 100%;
            margin-top: 10px;
        }
        button:hover {
            background-color: #e0a800;
        }
        .error-message {
            color: #dc3545;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 10px;
            border-radius: 4px;
            margin-bottom: 15px;
            text-align: center;
        }
        .success-message {
            color: #28a745;
            background-color: #d4edda;
            border: 1px solid #c3e6cb;
            padding: 10px;
            border-radius: 4px;
            margin-bottom: 15px;
            text-align: center;
        }
        .back-link {
            display: block;
            text-align: center;
            margin-top: 20px;
            color: #007bff;
            text-decoration: none;
        }
        .back-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Ubah Password</h1>

        <% if (error) { %>
            <p class="error-message"><%= error %></p>
        <% } %>
        <% if (success) { %>
            <p class="success-message"><%= success %></p>
        <% } %>

        <form action="/admin/change-password" method="POST">
            <div class="form-group">
                <label for="current_password">Password Saat Ini:</label>
                <input type="password" id="current_password" name="current_password" required>
            </div>

            <div class="form-group">
                <label for="new_password">Password Baru:</label>
                <input type="password" id="new_password" name="new_password" required>
            </div>

            <div class="form-group">
                <label for="confirm_password">Konfirmasi Password Baru:</label>
                <input type="password" id="confirm_password" name="confirm_password" required>
            </div>

            <button type="submit">Ubah Password</button>
        </form>

        <a href="/admin/users" class="back-link">Kembali ke Daftar Pengguna</a>
    </div>
</body>
</html>
```

`wms-app/views/admin/users/create.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Buat Pengguna Baru</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: flex-start; /* Align to top */
            min-height: 100vh;
        }
        .container {
            background-color: #fff;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 500px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 20px;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #555;
            font-weight: bold;
        }
        input[type="text"],
        input[type="email"],
        input[type="password"],
        select {
            width: calc(100% - 22px); /* Adjust for padding and border */
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box; /* Include padding and border in the element's total width and height */
            margin-top: 5px;
        }
        button {
            background-color: #007bff;
            color: white;
            padding: 12px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            width: 100%;
            margin-top: 10px;
        }
        button:hover {
            background-color: #0056b3;
        }
        .error-message {
            color: #dc3545;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 10px;
            border-radius: 4px;
            margin-bottom: 15px;
            text-align: center;
        }
        .back-link {
            display: block;
            text-align: center;
            margin-top: 20px;
            color: #007bff;
            text-decoration: none;
        }
        .back-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Buat Pengguna Baru</h1>

        <% if (error) { %>
            <p class="error-message"><%= error %></p>
        <% } %>

        <form action="/admin/users" method="POST">
            <div class="form-group">
                <label for="username">Username:</label>
                <input type="text" id="username" name="username" value="<%= formData ? formData.username : '' %>" required>
            </div>

            <div class="form-group">
                <label for="password">Password:</label>
                <input type="password" id="password" name="password" required>
            </div>

            <div class="form-group">
                <label for="full_name">Nama Lengkap:</label>
                <input type="text" id="full_name" name="full_name" value="<%= formData ? formData.full_name : '' %>" required>
            </div>

            <div class="form-group">
                <label for="email">Email:</label>
                <input type="email" id="email" name="email" value="<%= formData ? formData.email : '' %>" required>
            </div>

            <div class="form-group">
                <label for="role">Role:</label>
                <select id="role" name="role" required>
                    <option value="">Pilih Role</option>
                    <option value="admin" <%= formData && formData.role === 'admin' ? 'selected' : '' %>>Admin</option>
                    <option value="ppic" <%= formData && formData.role === 'ppic' ? 'selected' : '' %>>PPIC</option>
                    <option value="packing" <%= formData && formData.role === 'packing' ? 'selected' : '' %>>Packing</option>
                    <option value="mixing" <%= formData && formData.role === 'mixing' ? 'selected' : '' %>>Mixing</option>
                    <option value="milling" <%= formData && formData.role === 'milling' ? 'selected' : '' %>>Milling</option>
                    <option value="qc" <%= formData && formData.role === 'qc' ? 'selected' : '' %>>QC</option>
                    <option value="gudang" <%= formData && formData.role === 'gudang' ? 'selected' : '' %>>Gudang</option>
                </select>
            </div>

            <button type="submit">Buat Pengguna</button>
        </form>

        <a href="/admin/users" class="back-link">Kembali ke Daftar Pengguna</a>
    </div>
</body>
</html>
```

`wms-app/views/admin/users/edit.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Edit Pengguna</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: flex-start;
            min-height: 100vh;
        }
        .container {
            background-color: #fff;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 500px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 20px;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #555;
            font-weight: bold;
        }
        input[type="text"],
        input[type="email"],
        select {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
            margin-top: 5px;
        }
        input[type="checkbox"] {
            margin-top: 5px;
            margin-right: 10px;
        }
        button {
            background-color: #28a745; /* Green for update */
            color: white;
            padding: 12px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            width: 100%;
            margin-top: 10px;
        }
        button:hover {
            background-color: #218838;
        }
        .error-message {
            color: #dc3545;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 10px;
            border-radius: 4px;
            margin-bottom: 15px;
            text-align: center;
        }
        .back-link {
            display: block;
            text-align: center;
            margin-top: 20px;
            color: #007bff;
            text-decoration: none;
        }
        .back-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Edit Pengguna</h1>

        <% if (error) { %>
            <p class="error-message"><%= error %></p>
        <% } %>

        <form action="/admin/users/<%= user.id %>" method="POST">
            <div class="form-group">
                <label for="username">Username:</label>
                <input type="text" id="username" name="username" value="<%= user.username || '' %>" required>
            </div>

            <div class="form-group">
                <label for="full_name">Nama Lengkap:</label>
                <input type="text" id="full_name" name="full_name" value="<%= user.full_name || '' %>" required>
            </div>

            <div class="form-group">
                <label for="email">Email:</label>
                <input type="email" id="email" name="email" value="<%= user.email || '' %>" required>
            </div>

            <div class="form-group">
                <label for="role">Role:</label>
                <select id="role" name="role" required>
                    <option value="">Pilih Role</option>
                    <option value="admin" <%= user.role === 'admin' ? 'selected' : '' %>>Admin</option>
                    <option value="ppic" <%= user.role === 'ppic' ? 'selected' : '' %>>PPIC</option>
                    <option value="packing" <%= user.role === 'packing' ? 'selected' : '' %>>Packing</option>
                    <option value="mixing" <%= user.role === 'mixing' ? 'selected' : '' %>>Mixing</option>
                    <option value="milling" <%= user.role === 'milling' ? 'selected' : '' %>>Milling</option>
                    <option value="qc" <%= user.role === 'qc' ? 'selected' : '' %>>QC</option>
                    <option value="gudang" <%= user.role === 'gudang' ? 'selected' : '' %>>Gudang</option>
                </select>
            </div>

            <div class="form-group">
                <label for="is_active">Status Aktif:</label>
                <input type="checkbox" id="is_active" name="is_active" <%= user.is_active ? 'checked' : '' %>> Aktif
            </div>

            <button type="submit">Update Pengguna</button>
        </form>

        <a href="/admin/users" class="back-link">Kembali ke Daftar Pengguna</a>
    </div>
</body>
</html>
```

`wms-app/views/admin/users/list.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Manajemen Pengguna</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            margin: 0;
            padding: 20px;
            display: flex;
            flex-direction: column;
            align-items: center;
            min-height: 100vh;
        }
        .container {
            background-color: #fff;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 900px; /* Lebar yang lebih besar untuk tabel */
            margin-bottom: 20px;
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 20px;
        }
        .user-info {
            text-align: center;
            margin-bottom: 20px;
            color: #666;
        }
        .nav-links {
            text-align: center;
            margin-bottom: 25px;
        }
        .nav-links a {
            margin: 0 10px;
            text-decoration: none;
            color: #007bff;
            font-weight: bold;
        }
        .nav-links a:hover {
            text-decoration: underline;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }
        th, td {
            border: 1px solid #ddd;
            padding: 10px;
            text-align: left;
        }
        th {
            background-color: #f2f2f2;
            color: #333;
        }
        tr:nth-child(even) {
            background-color: #f9f9f9;
        }
        .action-buttons button {
            padding: 6px 12px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 14px;
            margin-right: 5px;
        }
        .edit-btn {
            background-color: #007bff;
            color: white;
        }
        .edit-btn:hover {
            background-color: #0056b3;
        }
        .delete-btn {
            background-color: #dc3545;
            color: white;
        }
        .delete-btn:hover {
            background-color: #c82333;
        }
        .message {
            color: #28a745;
            background-color: #d4edda;
            border: 1px solid #c3e6cb;
            padding: 10px;
            border-radius: 4px;
            margin-bottom: 15px;
            text-align: center;
        }
        .error-message {
            color: #dc3545;
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            padding: 10px;
            border-radius: 4px;
            margin-bottom: 15px;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Manajemen Pengguna</h1>

        <p class="user-info">Login sebagai: <strong><%= currentUser.fullName %> (<%= currentUser.role %>)</strong></p>

        <% if (message) { %>
            <p class="message"><%= message %></p>
        <% } %>
        <% if (error) { %>
            <p class="error-message"><%= error %></p>
        <% } %>

        <div class="nav-links">
            <a href="/admin/users/create">Tambah User Baru</a> |
            <a href="/admin/change-password">Ganti Password</a> |
            <a href="/logout">Logout</a>
        </div>

        <% if (users && users.length > 0) { %>
            <table>
                <thead>
                    <tr>
                        <th>Username</th>
                        <th>Nama Lengkap</th>
                        <th>Email</th>
                        <th>Role</th>
                        <th>Status</th>
                        <th>Aksi</th>
                    </tr>
                </thead>
                <tbody>
                    <% users.forEach(user => { %>
                        <tr>
                            <td><%= user.username %></td>
                            <td><%= user.full_name %></td>
                            <td><%= user.email %></td>
                            <td><%= user.role %></td>
                            <td><%= user.is_active ? 'Aktif' : 'Tidak Aktif' %></td>
                            <td class="action-buttons">
                                <button class="edit-btn" onclick="location.href='/admin/users/<%= user.id %>/edit'">Edit</button>
                                <% if (user.id !== currentUser.id) { %>
                                    <form action="/admin/users/<%= user.id %>/delete" method="POST" style="display:inline-block;">
                                        <button type="submit" class="delete-btn" onclick="return confirm('Apakah Anda yakin ingin men
ghapus pengguna <%= user.username %>?');">Delete</button>
                                    </form>
                                <% } %>
                            </td>
                        </tr>
                    <% }); %>
                </tbody>
            </table>
        <% } else { %>
            <p style="text-align: center;">Tidak ada pengguna yang ditemukan.</p>
        <% } %>
    </div>
</body>
</html>
```

`wms-app/views/dashboard/admin.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard Admin</title>
</head>
<body>
    <h1>Selamat Datang, Admin <%= user.fullName || user.username %>!</h1>
    <p>Anda login sebagai: **<%= user.role %>**</p>
    <a href="/logout">Logout</a>
    <p>Ini adalah halaman dashboard untuk Administrator. Anda memiliki akses penuh ke semua fitur.</p>
</body>
</html>
```

`wms-app/views/dashboard/gudang.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard PPIC - WMS App</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .user-info { color: #555; }
        .logout-link { display: inline-block; margin-top: 15px; padding: 8px 15px; background-color: #f44336; color: white; text-decoration: none; border-radius: 5px; }
        .logout-link:hover { background-color: #d32f2f; }
    </style>
</head>
<body>
    <h1>Selamat Datang, Dashboard gudang</h1>
    <% if (user) { %>
       <p class="user-info">Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong></p>
    <% } %>
    <p>Ini adalah halaman dashboard untuk role gudang. Anda akan menemukan fitur-fitur spesifik PPIC di sini.</p>

    <a href="/logout" class="logout-link">Logout</a>
</body>
</html>
```

`wms-app/views/dashboard/milling.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard PPIC - WMS App</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .user-info { color: #555; }
        .logout-link { display: inline-block; margin-top: 15px; padding: 8px 15px; background-color: #f44336; color: white; text-decoration: none; border-radius: 5px; }
        .logout-link:hover { background-color: #d32f2f; }
    </style>
</head>
<body>
    <h1>Selamat Datang, Dashboard milling</h1>
    <% if (user) { %>
       <p class="user-info">Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong></p>
    <% } %>
    <p>Ini adalah halaman dashboard untuk role milling. Anda akan menemukan fitur-fitur spesifik PPIC di sini.</p>

    <a href="/logout" class="logout-link">Logout</a>
</body>
</html>
```

`wms-app/views/dashboard/mixing.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard PPIC - WMS App</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .user-info { color: #555; }
        .logout-link { display: inline-block; margin-top: 15px; padding: 8px 15px; background-color: #f44336; color: white; text-decoration: none; border-radius: 5px; }
        .logout-link:hover { background-color: #d32f2f; }
    </style>
</head>
<body>
    <h1>Selamat Datang, Dashboard PPIC!</h1>
    <% if (user) { %>
       <p class="user-info">Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong></p>
    <% } %>
    <p>Ini adalah halaman dashboard untuk role mixing. Anda akan menemukan fitur-fitur spesifik PPIC di sini.</p>

    <a href="/logout" class="logout-link">Logout</a>
</body>
</html>
```

`wms-app/views/dashboard/packing.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard packing - WMS App</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .user-info { color: #555; }
        .logout-link { display: inline-block; margin-top: 15px; padding: 8px 15px; background-color: #f44336; color: white; text-decoration: none; border-radius: 5px; }
        .logout-link:hover { background-color: #d32f2f; }
    </style>
</head>
<body>
    <h1>Selamat Datang, Dashboard packing</h1>
    <% if (user) { %>
       <p class="user-info">Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong></p>
   <% } %>
    <p>Ini adalah halaman dashboard untuk role packing. Anda akan menemukan fitur-fitur spesifik PPIC di sini.</p>

    <a href="/logout" class="logout-link">Logout</a>
</body>
</html>
```

`wms-app/views/dashboard/ppic.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard PPIC - WMS App</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .user-info { color: #555; }
        .logout-link { display: inline-block; margin-top: 15px; padding: 8px 15px; background-color: #f44336; color: white; text-decoration: none; border-radius: 5px; }
        .logout-link:hover { background-color: #d32f2f; }
    </style>
</head>
<body>
    <h1>Selamat Datang, Dashboard PPIC!</h1>
    <% if (user) { %>
       <p class="user-info">Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong></p>
    <% } %>
    <p>Ini adalah halaman dashboard untuk role PPIC. Anda akan menemukan fitur-fitur spesifik PPIC di sini.</p>

    <a href="/logout" class="logout-link">Logout</a>
</body>
</html>
```

`wms-app/views/dashboard/qc.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard PPIC - WMS App</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .user-info { color: #555; }
        .logout-link { display: inline-block; margin-top: 15px; padding: 8px 15px; background-color: #f44336; color: white; text-decoration: none; border-radius: 5px; }
        .logout-link:hover { background-color: #d32f2f; }
    </style>
</head>
<body>
    <h1>Selamat Datang, Dashboard qc</h1>
    <% if (user) { %>
       <p class="user-info">Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong></p>
    <% } %>
    <p>Ini adalah halaman dashboard untuk role qc. Anda akan menemukan fitur-fitur spesifik PPIC di sini.</p>

    <a href="/logout" class="logout-link">Logout</a>
</body>
</html>
```

`wms-app/views/auth/forgot-password.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Lupa Password - WMS App</title>
</head>
<body>
    <div class="login-container">
        <h2>Lupa Password</h2>
        <% if (error) { %>
            <p class="error-message"><%= error %></p>
        <% } %>
        <% if (success) { %>
            <p class="success-message"><%= success %></p>
        <% } %>
        <form action="/forgot-password" method="POST">
            <label for="email">Email:</label>
            <input type="email" id="email" name="email" required>

            <button type="submit">Kirim Link Reset</button>
        </form>
        <p><a href="/login">Kembali ke login</a></p>
    </div>
</body>
</html>
```

`wms-app/views/auth/login.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login WMS App</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .login-container {
            background-color: #fff;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
            width: 300px;
            text-align: center;
        }
        .login-container h2 {
            margin-bottom: 20px;
            color: #333;
        }
        .login-container label {
            display: block;
            text-align: left;
            margin-bottom: 5px;
            color: #555;
        }
        .login-container input[type="text"],
        .login-container input[type="password"] {
            width: calc(100% - 20px); /* Kurangi padding dari width */
            padding: 10px;
            margin-bottom: 15px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        .login-container button {
            width: 100%;
            padding: 10px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
        }
        .login-container button:hover {
            background-color: #0056b3;
        }
        .error-message {
            color: red;
            margin-bottom: 15px;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="login-container">
        <h2>Login WMS</h2>
        <% if (error) { %>
            <p class="error-message"><%= error %></p>
        <% } %>
        <form action="/login" method="POST">
            <label for="username">Username:</label>
            <input type="text" id="username" name="username" required>

            <label for="password">Password:</label>
            <input type="password" id="password" name="password" required>

            <button type="submit">Login</button>
        </form>
    </div>
</body>
</html>
```

`wms-app/views/auth/register.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Register - WMS App</title>
    <style>
        /* CSS styling mirip dengan login */
    </style>
</head>
<body>
    <div class="login-container">
        <h2>Registrasi Akun Baru</h2>
        <% if (error) { %>
            <p class="error-message"><%= error %></p>
        <% } %>
        <form action="/register" method="POST">
            <label for="username">Username:</label>
            <input type="text" id="username" name="username" required>

            <label for="full_name">Nama Lengkap:</label>
            <input type="text" id="full_name" name="full_name" required>

            <label for="email">Email:</label>
            <input type="email" id="email" name="email" required>

            <label for="password">Password:</label>
            <input type="password" id="password" name="password" required>

            <label for="confirm_password">Konfirmasi Password:</label>
            <input type="password" id="confirm_password" name="confirm_password" required>

            <button type="submit">Daftar</button>
        </form>
        <p>Sudah punya akun? <a href="/login">Login disini</a></p>
    </div>
</body>
</html>
```

`wms-app/views/auth/register-success.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Registrasi Berhasil - WMS App</title>
</head>
<body>
    <h1>Registrasi Berhasil!</h1>
    <p>Email verifikasi telah dikirim ke <strong><%= email %></strong>. Silakan cek email Anda untuk verifikasi akun.</p>
    <p><a href="/login">Kembali ke halaman login</a></p>
</body>
</html>
```

`wms-app/views/auth/reset-password.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Reset Password - WMS App</title>
</head>
<body>
    <div class="login-container">
        <h2>Reset Password</h2>
        <% if (error) { %>
            <p class="error-message"><%= error %></p>
        <% } %>
        <% if (token) { %>
            <form action="/reset-password" method="POST">
                <input type="hidden" name="token" value="<%= token %>">
                
                <label for="password">Password Baru:</label>
                <input type="password" id="password" name="password" required>

                <label for="confirm_password">Konfirmasi Password Baru:</label>
                <input type="password" id="confirm_password" name="confirm_password" required>

                <button type="submit">Reset Password</button>
            </form>
        <% } else { %>
            <p>Link reset password tidak valid atau sudah kadaluarsa.</p>
            <p><a href="/forgot-password">Minta link reset password baru</a></p>
        <% } %>
    </div>
</body>
</html>
```

`wms-app/views/auth/reset-password-success.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Password Berhasil Direset - WMS App</title>
</head>
<body>
    <h1>Password Berhasil Direset!</h1>
    <p>Password Anda telah berhasil diubah. Silakan login dengan password baru Anda.</p>
    <p><a href="/login">Login sekarang</a></p>
</body>
</html>
```

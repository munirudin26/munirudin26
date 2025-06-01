### Membangun Modul Autentikasi Pengguna (User Login) dengan Node.js, Express, EJS, JWT, dan MySQL

Dokumen ini akan memandu Anda langkah demi langkah dalam membuat sistem autentikasi pengguna dasar menggunakan kombinasi teknologi Node.js, Express.js (sebagai backend), EJS (templating engine untuk frontend), JSON Web Tokens (JWT) untuk sesi, dan MySQL sebagai database.

Tujuan dari modul ini adalah untuk memungkinkan pengguna login ke sistem dan kemudian diarahkan ke dashboard yang sesuai dengan peran (role) mereka.

# 1. Persiapan Proyek

Kita akan memulai dengan menyiapkan lingkungan proyek Node.js Anda.

1.1. Inisialisasi Proyek

Buat folder proyek:
```
mkdir wms-app
cd wms-app
```

Inisialisasi proyek Node.js: 

Ini akan membuat file package.json yang mengelola dependensi proyek Anda.
```
npm init -y
```

1.2. Instalasi Dependensi

Instal semua package Node.js yang diperlukan untuk proyek ini.
```
npm install express mysql2 ejs bcryptjs jsonwebtoken dotenv cookie-parser
```

`express`: Web framework untuk membangun API dan route.

`mysql2`: Driver MySQL yang efisien untuk koneksi database.

`ejs`: Templating engine untuk menyajikan halaman web.

`bcryptjs`: Untuk hashing kata sandi agar disimpan dengan aman di database.

`jsonwebtoken`: Untuk mengimplementasikan JSON Web Tokens (JWT) sebagai metode autentikasi sesi.

`dotenv`: Untuk memuat variabel lingkungan dari file `.env`.

`cookie-parser`: Middleware untuk mem-parsing HTTP cookies yang dikirim dengan permintaan.

1.3. Konfigurasi Environment Variables (`.env`)

Buat file bernama `.env` di root folder proyek Anda (`wms-app/`) dan tambahkan konfigurasi database serta secret key untuk JWT. Ganti nilai placeholder dengan informasi Anda yang sebenarnya.
```
# .env
DB_HOST=localhost           # Atau 127.0.0.1 jika localhost tidak berfungsi
DB_USER=root                # Username MySQL Anda
DB_PASSWORD=your_mysql_password # Password MySQL Anda
DB_NAME=wms_db              # Nama database yang akan digunakan
JWT_SECRET=supersecretjwtkey_ganti_ini_nanti # Ganti dengan string acak dan kompleks!
PORT=3000                   # Port untuk menjalankan aplikasi Node.js
```

Penting: 

Variabel `JWT_SECRET` harus sangat kuat dan dirahasiakan. Jangan pernah meng-hardcode nilai ini langsung di kode.

# 2. Persiapan Database MySQL

Sebelum menjalankan aplikasi Node.js, kita perlu memastikan database dan tabel `users` sudah tersedia di MySQL.

2.1. Memulai MySQL Server

Pastikan server MySQL Anda berjalan. Cara memulainya bervariasi tergantung sistem operasi dan cara Anda menginstal MySQL (misal: XAMPP/WAMP di Windows, Homebrew di macOS, `systemctl` di Linux).

2.2. Membuat Database dan Tabel users

Buka terminal atau MySQL Client (contoh: MySQL Workbench, DBeaver, phpMyAdmin).

Login ke MySQL server:
```
mysql -u root -p
# Masukkan password MySQL Anda saat diminta
```

Buat database `wms_db`:
```
CREATE DATABASE IF NOT EXISTS wms_db;
```

Pilih database `wms_db`:
```
USE wms_db;
```

Buat tabel `users`:
```
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
```

2.3. Menambahkan User Admin Awal (Direkomendasikan)

Untuk dapat langsung login dan menguji aplikasi, kita akan menambahkan satu pengguna admin secara manual.

Generate Hash Password: 

Kita perlu hash password `admin123` menggunakan `bcryptjs`.

Buat file sementara (misal: `hash_password.js`) di root folder proyek `wms-app` Anda:
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

hashPassword();
```

```
// hash_password.js
const bcrypt = require('bcryptjs');

const saltRounds = 10; // Jumlah salt rounds, semakin tinggi semakin aman tapi semakin lambat

async function hashPassword(password) {
    try {
        const salt = await bcrypt.genSalt(saltRounds);
        const hash = await bcrypt.hash(password, salt);
        console.log(`Password: ${password}`);
        console.log(`Hashed Password: ${hash}`);
        return hash;
    } catch (err) {
        console.error('Error hashing password:', err);
        throw err;
    }
}

// Contoh penggunaan:
// Untuk menjalankan ini, cukup buka terminal di root folder project dan jalankan:
// node hash_password.js "your_admin_password_here"

const passwordToHash = process.argv[2]; // Ambil password dari argumen command line

if (passwordToHash) {
    hashPassword(passwordToHash);
} else {
    console.log('Usage: node hash_password.js <password_to_hash>');
}
```

Jalankan file tersebut menggunakan Node.js di terminal:
```
node hash_password.js
```

Salin hash password yang dihasilkan (contoh: $2a$10$T89nXznWjyDwrV38I3K7GeKcernBpk1cfekz72wRBRwdaWpte7xHi).

Insert User ke Database:

Kembali ke MySQL Client.

Jalankan perintah SQL ini. Ganti hash password dengan yang Anda salin tadi.
```
INSERT IGNORE INTO users (username, password_hash, full_name, email, role)
VALUES ('admin', 'PASTE_HASH_PASSWORD_ANDA_DI_SINI', 'Administrator', 'admin@wms.com', 'admin');
```

Keluar dari MySQL Client:
```
EXIT;
```

# 3. Struktur Folder Proyek

Pastikan struktur folder proyek Anda mirip dengan ini:
```
wms-app/
├── node_modules/
├── src/
│   ├── config/             # Konfigurasi database
│   │   └── database.js
│   ├── controllers/        # Logika bisnis (login, logout, dll.)
│   │   └── authController.js
│   ├── models/             # Interaksi dengan database (User model)
│   │   └── User.js
│   ├── routes/             # Definisi API endpoints (auth routes)
│   │   └── authRoutes.js
│   ├── middleware/         # Middleware untuk autentikasi & otorisasi
│   │   └── authMiddleware.js
│   └── app.js              # File utama aplikasi Express
├── views/                  # Folder untuk template EJS
│   ├── auth/
│   │   └── login.ejs
│   └── dashboard/
│       ├── admin.ejs       # Contoh dashboard admin
│       ├── ppic.ejs        # Contoh dashboard ppic
│       └── ... (dan untuk role lainnya)
├── .env                    # Environment variables
├── .gitignore
└── package.json
```

# 4. Implementasi Kode

Sekarang, mari kita tulis kode untuk setiap komponen.

4.1. Konfigurasi Database (`src/config/database.js`)
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

4.2. Model User (`src/models/User.js`)
```
// src/models/User.js
const { pool } = require('../config/database');

class User {
    static async findByUsername(username) {
        const [rows] = await pool.query('SELECT * FROM users WHERE username = ?', [username]);
        return rows[0]; // Mengembalikan baris pertama jika ditemukan
    }

    static async findById(id) {
        const [rows] = await pool.query('SELECT * FROM users WHERE id = ?', [id]);
        return rows[0];
    }
}

module.exports = User;
```

Controller Autentikasi (`src/controllers/authController.js`)
```
// src/controllers/authController.js
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const User = require('../models/User');
require('dotenv').config(); // Memuat variabel lingkungan

exports.showLoginForm = (req, res) => {
    // Merender halaman login EJS, mengirimkan variabel error (jika ada)
    res.render('auth/login', { error: null });
};

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
```

4.4. Middleware Autentikasi (`src/middleware/authMiddleware.js`)
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

Catatan: 

Anda perlu membuat file `views/error.ejs` sederhana untuk menampilkan pesan error jika akses ditolak:
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

4.5. Route Autentikasi (`src/routes/authRoutes.js`)
```
// src/routes/authRoutes.js
const express = require('express');
const router = express.Router();
const authController = require('../controllers/authController');

// Route untuk menampilkan form login (GET request)
router.get('/login', authController.showLoginForm);
// Route untuk memproses login (POST request)
router.post('/login', authController.login);
// Route untuk logout
router.get('/logout', authController.logout);

module.exports = router;
```

4.6. Main Aplikasi (`src/app.js`)
```
// src/app.js
const express = require('express');
const path = require('path');
const cookieParser = require('cookie-parser');
const { connectDB } = require('./config/database');
const authRoutes = require('./routes/authRoutes');
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
            return res.render('dashboard', { user: req.user, message: 'Selamat datang di WMS!' });
    }
});

// Contoh Route Dashboard yang Dilindungi per Role
// Setiap role memiliki dashboardnya sendiri yang hanya bisa diakses oleh role tersebut (dan admin)
app.get('/dashboard/admin', authenticateToken, authorizeRoles('admin'), (req, res) => {
    res.render('dashboard/admin', { user: req.user });
});

app.get('/dashboard/ppic', authenticateToken, authorizeRoles('admin', 'ppic'), (req, res) => {
    res.render('dashboard/ppic', { user: req.user });
});

app.get('/dashboard/packing', authenticateToken, authorizeRoles('admin', 'packing'), (req, res) => {
    res.render('dashboard/packing', { user: req.user });
});

app.get('/dashboard/mixing', authenticateToken, authorizeRoles('admin', 'mixing'), (req, res) => {
    res.render('dashboard/mixing', { user: req.user });
});

app.get('/dashboard/milling', authenticateToken, authorizeRoles('admin', 'milling'), (req, res) => {
    res.render('dashboard/milling', { user: req.user });
});

app.get('/dashboard/qc', authenticateToken, authorizeRoles('admin', 'qc'), (req, res) => {
    res.render('dashboard/qc', { user: req.user });
});

app.get('/dashboard/gudang', authenticateToken, authorizeRoles('admin', 'gudang'), (req, res) => {
    res.render('dashboard/gudang', { user: req.user });
});

// Jalankan server
app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
```

4.7. Tampilan EJS (`views/`)

Buat file-file EJS berikut di dalam folder `views` Anda.

`views/auth/login.ejs` (Form Login)
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

Contoh Dashboard (`views/dashboard/admin.ejs`)

Buat folder `views/dashboard/` dan di dalamnya buat file `admin.ejs`, `ppic.ejs`, `packing.ejs`, `mixing.ejs`, `milling.ejs`, `qc.ejs`, dan `gudang.ejs`. Isinya bisa sederhana seperti `admin.ejs` ini:
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

# 5. Cara Menjalankan Aplikasi

Pastikan server MySQL Anda berjalan.

Buka terminal di root folder proyek `wms-app`.

Jalankan aplikasi Node.js:
```
node src/app.js
```

Anda akan melihat output seperti `Connected to MySQL database! dan Server is running on http://localhost:3000`

Buka browser Anda dan navigasi ke http://localhost:3000/login.

Coba login:

Username: admin

Password: admin123 (atau password lain yang Anda hash dan masukkan ke database)

Verifikasi Sukses Login:

Jika berhasil, Anda akan di-redirect ke `http://localhost:3000/dashboard/admin`

# Pemecahan Masalah Umum

`Error: Cannot find module 'cookie-parser'` atau modul lainnya:

Ini berarti dependensi belum terinstal. Jalankan `npm install` kembali di root folder proyek.

Jika sudah, coba hapus `node_modules` dan `package-lock.json`, lalu `npm install` lagi:
```
rm -rf node_modules
rm -f package-lock.json
npm cache clean --force
npm install
```

`Error connecting to database: connect ECONNREFUSED ::1:3306:`

MySQL Server Tidak Berjalan: Pastikan server MySQL Anda sudah di-start.

Konfigurasi `.env` Salah: Periksa `DB_HOST`, `DB_USER`, `DB_PASSWORD`, dan `DB_NAME` di file `.env`. Coba ubah `DB_HOST=localhost` menjadi `DB_HOST=127.0.0.1`.

Firewall: Jarang terjadi untuk `localhost`, tapi pastikan firewall tidak memblokir port `3306`.

Kredensial MySQL: Coba login ke MySQL secara manual (`mysql -u root -p`) untuk memverifikasi kredensial dan koneksi.

`"Username atau password salah." setelah input login`:

Pastikan Anda menginput username dan password yang benar.

Verifikasi bahwa hash password di database untuk `admin` cocok dengan hash dari `admin123` (atau password yang Anda gunakan). Anda bisa mengeceknya langsung di MySQL: `SELECT username, password_hash FROM users WHERE username = 'admin';`

## Sekarang, ada banyak arah pengembangan yang bisa Anda lakukan untuk memperluas fungsionalitas WMS Anda. Berikut adalah beberapa ide pengembangan yang bisa Anda pertimbangkan, dibagi per modul utama:

### A. Modul Manajemen Inventaris (Core WMS Functionality)

Ini adalah jantung dari setiap sistem WMS.

### Manajemen Item/Produk:

    1. CRUD (Create, Read, Update, Delete) Item: Halaman untuk menambah, melihat, mengedit, dan menghapus detail produk (nama, SKU, deskripsi, kategori, unit pengukuran, harga beli/jual).
    
    2. Kategori Produk: Fitur untuk mengelola kategori produk untuk pengorganisasian yang lebih baik.
    
    3. Unit Pengukuran: Definisi unit pengukuran (kg, pcs, box, liter, dll.).

### Manajemen Lokasi Gudang (Warehousing/Location Management):

    1. Definisi Lokasi: Fitur untuk membuat dan mengelola lokasi penyimpanan (misalnya, Aisle 1, Rack B, Level 3, Bin 4 atau Zone A, Shelf B).

    2. Tipe Lokasi: Kemampuan untuk mengidentifikasi tipe lokasi (misalnya, picking, bulk storage, staging, receiving).

    3. Kapasitas Lokasi: Menetapkan kapasitas maksimum untuk setiap lokasi.

### Manajemen Stok (Inventory Management):

    1. Stok Aktual: Tampilan real-time dari jumlah stok untuk setiap item di setiap lokasi.

    2. Pergerakan Stok (Stock Movement):
      
      -Stock In (Penerimaan Barang): Pencatatan barang masuk ke gudang (dari supplier, produksi, dll.), dengan menentukan kuantitas dan lokasi penyimpanan.

      -Stock Out (Pengeluaran Barang): Pencatatan barang keluar dari gudang (untuk pengiriman, produksi, transfer, dll.).
      
      -Transfer Stok: Memindahkan stok dari satu lokasi ke lokasi lain di dalam gudang.

    3. Penyesuaian Stok (Stock Adjustment): Fitur untuk menyesuaikan jumlah stok karena kehilangan, kerusakan, atau temuan saat cycle counting.

    4. History Stok: Log semua transaksi pergerakan stok untuk auditing.

### Mengapa urutan ini?

    1. Item & Lokasi adalah Master Data: Anda tidak bisa menyimpan stok atau memindahkan barang jika Anda belum mendefinisikan apa yang disimpan (item) dan di mana menyimpannya (lokasi).

    2. Stok adalah Konsekuensi: Setelah item dan lokasi ada, baru kita bisa mulai melacak stok dan pergerakannya.

### Langkah Pertama:

Kita akan mulai dengan Manajemen Item/Produk. Ini akan melibatkan:

    1. Membuat tabel `products` di database.

    2. Membuat Model `Product.js` untuk interaksi dengan tabel `products`.

    3. Membuat Controller `productController.js` untuk logika bisnis CRUD.

    4. Menambahkan rute di `adminRoutes.js` untuk manajemen produk (misalnya, `/admin/products`).

    5. Membuat view EJS untuk daftar produk (`list.ejs`), tambah produk (`create.ejs`), dan edit produk (`edit.ejs`).

### membuat tabel `products` di database MySQL Anda.
```
mysql -u root -p
```
```
use wms_db
```
```
CREATE TABLE IF NOT EXISTS products (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sku VARCHAR(50) UNIQUE NOT NULL, -- Stock Keeping Unit, identifikasi unik produk
    name VARCHAR(255) NOT NULL,
    description TEXT,
    category VARCHAR(100), -- Kategori produk (misal: Elektronik, Makanan, Bahan Baku)
    unit_of_measure VARCHAR(50) NOT NULL, -- Satuan unit (misal: PCS, KG, Liter, Box)
    current_stock_quantity DECIMAL(10, 2) DEFAULT 0, -- Jumlah stok saat ini (bisa desimal)
    min_stock_level DECIMAL(10, 2) DEFAULT 0, -- Level stok minimum untuk peringatan
    max_stock_level DECIMAL(10, 2) DEFAULT NULL, -- Level stok maksimum (opsional)
    location_default VARCHAR(255), -- Lokasi penyimpanan default (bisa diubah nanti dengan tabel locations)
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);
```

-- Opsional: Tambahkan indeks untuk pencarian cepat
```
CREATE INDEX idx_products_sku ON products (sku);
CREATE INDEX idx_products_name ON products (name);
CREATE INDEX idx_products_category ON products (category);
```

Penjelasan Kolom:

    id: Kunci utama untuk identifikasi unik setiap produk.

    sku: (Stock Keeping Unit) Kode unik produk. Ini sangat penting dalam WMS. Saya sarankan UNIQUE dan NOT NULL.

    name: Nama produk.

    description: Deskripsi singkat produk.

    category: Kategori produk. Anda bisa membuatnya sebagai ENUM jika kategori sudah tetap, atau VARCHAR untuk fleksibilitas.

    unit_of_measure: Satuan unit produk (misalnya, PCS, KG, Liter, BOX). Ini juga bisa menjadi ENUM.

    current_stock_quantity: Jumlah stok produk saat ini. Menggunakan DECIMAL untuk kuantitas yang mungkin tidak selalu bilangan bulat (misal: 0.5 kg). Default 0.

    min_stock_level: Level stok minimum. Ketika stok mencapai atau di bawah level ini, sistem bisa memberikan peringatan.

    max_stock_level: Level stok maksimum yang ideal. (Opsional)

    location_default: Sebuah kolom awal untuk menunjukkan lokasi default produk. Nantinya, ini akan kita ganti dengan tabel locations yang terpisah dan sistem stok yang lebih kompleks.

    is_active: Status aktif/nonaktif produk.

    created_at, updated_at: Timestamp untuk melacak kapan data dibuat dan terakhir diperbarui.    

### `src/models/Product.js`
```
// src/models/Product.js
const { pool } = require('../config/database');

class Product {
    // Mengambil semua produk
    static async getAll() {
        const [rows] = await pool.query('SELECT * FROM products ORDER BY name ASC');
        return rows;
    }

    // Mengambil produk berdasarkan ID
    static async findById(id) {
        const [rows] = await pool.query('SELECT * FROM products WHERE id = ?', [id]);
        return rows[0];
    }

    // Mengambil produk berdasarkan SKU
    static async findBySku(sku) {
        const [rows] = await pool.query('SELECT * FROM products WHERE sku = ?', [sku]);
        return rows[0];
    }

    // Membuat produk baru
    static async create({ sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default }) {
        const [result] = await pool.query(
            'INSERT INTO products (sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default) VALUES (?, ?, ?, ?, ?, ?, ?, ?)',
            [sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default]
        );
        return result.insertId; // Mengembalikan ID produk yang baru dibuat
    }

    // Memperbarui produk berdasarkan ID
    static async update(id, { sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default, is_active }) {
        const [result] = await pool.query(
            'UPDATE products SET sku = ?, name = ?, description = ?, category = ?, unit_of_measure = ?, min_stock_level = ?, max_stock_level = ?, location_default = ?, is_active = ? WHERE id = ?',
            [sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default, is_active, id]
        );
        return result.affectedRows; // Mengembalikan jumlah baris yang terpengaruh
    }

    // Menghapus produk berdasarkan ID
    static async delete(id) {
        const [result] = await pool.query('DELETE FROM products WHERE id = ?', [id]);
        return result.affectedRows; // Mengembalikan jumlah baris yang terpengaruh
    }

    // Memperbarui kuantitas stok saat ini (akan disempurnakan nanti dengan pergerakan stok)
    static async updateStockQuantity(id, newQuantity) {
        const [result] = await pool.query('UPDATE products SET current_stock_quantity = ? WHERE id = ?', [newQuantity, id]);
        return result.affectedRows;
    }
}

module.exports = Product;
```

Penjelasan Kode:

    getAll(): Mengambil semua record produk dari tabel products.

    findById(id): Mencari dan mengembalikan satu produk berdasarkan id nya.

    findBySku(sku): Mencari dan mengembalikan satu produk berdasarkan sku (ini akan sangat penting).

    create(...): Menyisipkan data produk baru ke tabel.

    update(...): Memperbarui data produk yang sudah ada.

    delete(id): Menghapus produk dari tabel.

    updateStockQuantity(id, newQuantity): Metode dasar untuk memperbarui stok. (Catatan: Ini adalah metode sementara; nanti kita akan mengimplementasikan pergerakan stok yang lebih canggih).

### `src/controllers/productController.js`
```
// src/controllers/productController.js
const Product = require('../models/Product'); // Import model Product
const { pool } = require('../config/database'); // Untuk query langsung jika diperlukan, atau hanya untuk error handling

// Fungsi untuk menampilkan daftar produk
exports.listProducts = async (req, res) => {
    try {
        const products = await Product.getAll(); // Ambil semua produk dari database
        const message = req.query.message || null; // Ambil pesan sukses dari query parameter
        const error = req.query.error || null; // Ambil pesan error dari query parameter

        res.render('admin/products/list', { // Render view list.ejs di folder admin/products
            products,
            user: req.user, // Pastikan user login diteruskan ke view
            message: message,
            error: error
        });
    } catch (err) {
        console.error('Error fetching products:', err);
        res.render('error', { message: 'Gagal mengambil data produk.' });
    }
};

// Fungsi untuk menampilkan form tambah produk baru
exports.showCreateProductForm = (req, res) => {
    res.render('admin/products/create', {
        user: req.user,
        error: null // Awalnya tidak ada error
    });
};

// Fungsi untuk memproses penambahan produk baru
exports.createProduct = async (req, res) => {
    const { sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default } = req.body;

    // Validasi input dasar
    if (!sku || !name || !unit_of_measure) {
        return res.render('admin/products/create', {
            user: req.user,
            error: 'SKU, Nama Produk, dan Satuan Unit harus diisi.'
        });
    }

    try {
        // Cek apakah SKU sudah ada
        const existingProduct = await Product.findBySku(sku);
        if (existingProduct) {
            return res.render('admin/products/create', {
                user: req.user,
                error: `SKU "${sku}" sudah ada. Gunakan SKU lain.`
            });
        }

        // Buat produk baru
        await Product.create({
            sku,
            name,
            description: description || null, // Jika kosong, set null
            category: category || null,
            unit_of_measure,
            min_stock_level: min_stock_level || 0, // Default 0 jika kosong
            max_stock_level: max_stock_level || null,
            location_default: location_default || null
        });

        res.redirect('/admin/products?message=Produk berhasil ditambahkan!');
    } catch (err) {
        console.error('Error creating product:', err);
        res.render('admin/products/create', {
            user: req.user,
            error: 'Gagal menambahkan produk. Terjadi kesalahan server.'
        });
    }
};

// Fungsi untuk menampilkan form edit produk
exports.showEditProductForm = async (req, res) => {
    try {
        const product = await Product.findById(req.params.id);
        if (!product) {
            return res.redirect('/admin/products?error=Produk tidak ditemukan.');
        }
        res.render('admin/products/edit', {
            product,
            user: req.user,
            error: null
        });
    } catch (err) {
        console.error('Error fetching product for edit:', err);
        res.render('error', { message: 'Gagal mengambil data produk untuk diedit.' });
    }
};

// Fungsi untuk memproses pembaruan produk
exports.updateProduct = async (req, res) => {
    const { id } = req.params;
    const { sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default, is_active } = req.body;
    const isActiveBoolean = is_active === 'true'; // Konversi string 'true'/'false' dari checkbox menjadi boolean

    // Validasi input dasar
    if (!sku || !name || !unit_of_measure) {
        return res.redirect(`/admin/products/${id}/edit?error=SKU, Nama Produk, dan Satuan Unit harus diisi.`);
    }

    try {
        // Cek apakah SKU baru sudah digunakan oleh produk lain (kecuali produk ini sendiri)
        const existingProduct = await Product.findBySku(sku);
        if (existingProduct && existingProduct.id !== parseInt(id)) {
            return res.redirect(`/admin/products/${id}/edit?error=SKU "${sku}" sudah digunakan oleh produk lain.`);
        }

        await Product.update(id, {
            sku,
            name,
            description: description || null,
            category: category || null,
            unit_of_measure,
            min_stock_level: min_stock_level || 0,
            max_stock_level: max_stock_level || null,
            location_default: location_default || null,
            is_active: isActiveBoolean
        });

        res.redirect('/admin/products?message=Produk berhasil diperbarui!');
    } catch (err) {
        console.error('Error updating product:', err);
        res.redirect(`/admin/products/${id}/edit?error=Gagal memperbarui produk. Terjadi kesalahan server.`);
    }
};

// Fungsi untuk memproses penghapusan produk
exports.deleteProduct = async (req, res) => {
    const { id } = req.params;
    try {
        // Cek apakah produk memiliki stok (jika ada, mungkin tidak boleh dihapus langsung)
        // Untuk saat ini, kita izinkan hapus. Nanti bisa ditambahkan validasi ini.
        const product = await Product.findById(id);
        if (!product) {
            return res.redirect('/admin/products?error=Produk tidak ditemukan.');
        }

        await Product.delete(id);
        res.redirect('/admin/products?message=Produk berhasil dihapus!');
    } catch (err) {
        console.error('Error deleting product:', err);
        res.redirect('/admin/products?error=Gagal menghapus produk. Terjadi kesalahan server.');
    }
};
```

Penjelasan Kode:

    Import Model Product: Menggunakan model yang baru saja kita buat untuk berinteraksi dengan database.

    listProducts: Mengambil semua produk dari database dan merender view admin/products/list.ejs. Meneruskan products, user, dan pesan message/error (dari query parameter setelah redirect) ke view.

    showCreateProductForm: Merender view admin/products/create.ejs untuk form penambahan produk.

    createProduct: Menerima data dari form POST. Melakukan validasi dasar (misal: sku, name, unit_of_measure tidak boleh kosong) dan memeriksa sku duplikat sebelum menyimpan produk ke database. Setelah berhasil, akan redirect ke /admin/products dengan pesan sukses.

    showEditProductForm: Mengambil data produk berdasarkan ID dari URL parameter, lalu merender view admin/products/edit.ejs dengan data produk tersebut.

    updateProduct: Menerima data dari form POST. Melakukan validasi dan memeriksa sku duplikat (untuk produk lain). Memperbarui data produk di database.

    deleteProduct: Menghapus produk dari database berdasarkan ID.

### `src/routes/adminRoutes.js`

    Tambahkan import productController di bagian atas file.

    Tambahkan rute-rute baru untuk produk. Pastikan rute-rute ini berada setelah authorizeRoles('admin') atau di dalam blok router yang sudah dilindungi oleh middleware tersebut.
    
```
// src/routes/adminRoutes.js
const express = require('express');
const router = express.Router();
const userController = require('../controllers/userController');
const { authenticateToken, authorizeRoles } = require('../middleware/authMiddleware');
const productController = require('../controllers/productController'); // Pastikan ini ada dan benar

// Middleware untuk semua rute admin: memastikan user terautentikasi dan memiliki role 'admin'
router.use(authenticateToken);
router.use(authorizeRoles('admin'));

// --- Rute untuk Manajemen Pengguna (User Management) ---
router.get('/users', userController.listUsers);
router.get('/users/create', userController.showCreateUserForm);
router.post('/users/create', userController.createUser); // <-- PERBAIKI INI (sesuai /users/create)
router.get('/users/:id/edit', userController.showEditUserForm);
router.post('/users/:id/edit', userController.updateUser); // <-- PERBAIKI INI (sesuai /users/:id/edit)
router.post('/users/:id/delete', userController.deleteUser); // Ini sudah benar

// --- Rute untuk Ganti Password Admin ---
router.get('/change-password', userController.showChangePasswordForm);
router.post('/change-password', userController.changePassword);

// --- Rute BARU untuk Manajemen Produk (Product Management) ---
// Tampilkan daftar produk
router.get('/products', productController.listProducts);
// Tampilkan form tambah produk
router.get('/products/create', productController.showCreateProductForm);
// Proses tambah produk
router.post('/products/create', productController.createProduct);
// Tampilkan form edit produk
router.get('/products/:id/edit', productController.showEditProductForm);
// Proses update produk
router.post('/products/:id/edit', productController.updateProduct);
// Proses hapus produk
router.post('/products/:id/delete', productController.deleteProduct);


module.exports = router;
```

### `wms-app/views/admin/products/list.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Manajemen Produk - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 90%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; }
        .header-controls { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
        .header-controls a { background-color: #007bff; color: white; padding: 10px 15px; text-decoration: none; border-radius: 5px; }
        .header-controls a:hover { background-color: #0056b3; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        table { width: 100%; border-collapse: collapse; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
        .actions a, .actions button {
            padding: 5px 10px;
            text-decoration: none;
            border-radius: 3px;
            margin-right: 5px;
            font-size: 0.9em;
        }
        .actions a.edit-btn { background-color: #ffc107; color: black; }
        .actions a.edit-btn:hover { background-color: #e0a800; }
        .actions button.delete-btn { background-color: #dc3545; color: white; border: none; cursor: pointer; }
        .actions button.delete-btn:hover { background-color: #c82333; }
        .status-active { color: green; font-weight: bold; }
        .status-inactive { color: red; font-weight: bold; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <div class="header-controls">
            <h1>Daftar Produk</h1>
            <a href="/admin/products/create">Tambah Produk Baru</a>
        </div>

        <% if (message) { %>
            <div class="messages message-success"><%= message %></div>
        <% } %>
        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <table>
            <thead>
                <tr>
                    <th>ID</th>
                    <th>SKU</th>
                    <th>Nama Produk</th>
                    <th>Kategori</th>
                    <th>Unit Ukur</th>
                    <th>Stok Saat Ini</th>
                    <th>Min. Stok</th>
                    <th>Lokasi Default</th>
                    <th>Status</th>
                    <th>Aksi</th>
                </tr>
            </thead>
            <tbody>
                <% if (products && products.length > 0) { %>
                    <% products.forEach(product => { %>
                        <tr>
                            <td><%= product.id %></td>
                            <td><%= product.sku %></td>
                            <td><%= product.name %></td>
                            <td><%= product.category || '-' %></td>
                            <td><%= product.unit_of_measure %></td>
                            <td><%= product.current_stock_quantity %></td>
                            <td><%= product.min_stock_level %></td>
                            <td><%= product.location_default || '-' %></td>
                            <td><span class="<%= product.is_active ? 'status-active' : 'status-inactive' %>">
                                <%= product.is_active ? 'Aktif' : 'Nonaktif' %>
                            </span></td>
                            <td class="actions">
                                <a href="/admin/products/<%= product.id %>/edit" class="edit-btn">Edit</a>
                                <form action="/admin/products/<%= product.id %>/delete" method="POST" style="display:inline;" onsubmit="return confirm('Anda yakin ingin menghapus produk ini?');">
                                    <button type="submit" class="delete-btn">Hapus</button>
                                </form>
                            </td>
                        </tr>
                    <% }); %>
                <% } else { %>
                    <tr>
                        <td colspan="10">Belum ada produk yang terdaftar.</td>
                    </tr>
                <% } %>
            </tbody>
        </table>
    </div>
</body>
</html>
```

### `wms-app/views/admin/products/create.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Tambah Produk Baru - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 80%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; margin-bottom: 20px; }
        form { display: grid; grid-template-columns: 1fr 2fr; gap: 15px; align-items: center; }
        label { font-weight: bold; }
        input[type="text"], input[type="number"], textarea, select {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        textarea { resize: vertical; min-height: 80px; }
        button {
            grid-column: 1 / 3;
            padding: 10px 15px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover { background-color: #0056b3; }
        .back-link { display: block; margin-top: 20px; text-decoration: none; color: #007bff; }
        .back-link:hover { text-decoration: underline; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <h1>Tambah Produk Baru</h1>

        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <form action="/admin/products/create" method="POST">
            <label for="sku">SKU:</label>
            <input type="text" id="sku" name="sku" required>

            <label for="name">Nama Produk:</label>
            <input type="text" id="name" name="name" required>

            <label for="description">Deskripsi:</label>
            <textarea id="description" name="description"></textarea>

            <label for="category">Kategori:</label>
            <input type="text" id="category" name="category">

            <label for="unit_of_measure">Satuan Unit:</label>
            <input type="text" id="unit_of_measure" name="unit_of_measure" required placeholder="Contoh: PCS, KG, Liter">

            <label for="min_stock_level">Min. Stok Level:</label>
            <input type="number" id="min_stock_level" name="min_stock_level" value="0" step="any">

            <label for="max_stock_level">Max. Stok Level:</label>
            <input type="number" id="max_stock_level" name="max_stock_level" step="any">

            <label for="location_default">Lokasi Default:</label>
            <input type="text" id="location_default" name="location_default">

            <button type="submit">Tambah Produk</button>
        </form>
        <a href="/admin/products" class="back-link">Kembali ke Daftar Produk</a>
    </div>
</body>
</html>
```

### `wms-app/views/admin/products/edit.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Edit Produk - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 80%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; margin-bottom: 20px; }
        form { display: grid; grid-template-columns: 1fr 2fr; gap: 15px; align-items: center; }
        label { font-weight: bold; }
        input[type="text"], input[type="number"], textarea, select {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        textarea { resize: vertical; min-height: 80px; }
        button {
            grid-column: 1 / 3;
            padding: 10px 15px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover { background-color: #0056b3; }
        .back-link { display: block; margin-top: 20px; text-decoration: none; color: #007bff; }
        .back-link:hover { text-decoration: underline; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <h1>Edit Produk</h1>

        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <% if (product) { %>
            <form action="/admin/products/<%= product.id %>/edit" method="POST">
                <label for="sku">SKU:</label>
                <input type="text" id="sku" name="sku" value="<%= product.sku %>" required>

                <label for="name">Nama Produk:</label>
                <input type="text" id="name" name="name" value="<%= product.name %>" required>

                <label for="description">Deskripsi:</label>
                <textarea id="description" name="description"><%= product.description || '' %></textarea>

                <label for="category">Kategori:</label>
                <input type="text" id="category" name="category" value="<%= product.category || '' %>">

                <label for="unit_of_measure">Satuan Unit:</label>
                <input type="text" id="unit_of_measure" name="unit_of_measure" value="<%= product.unit_of_measure %>" required>

                <label for="min_stock_level">Min. Stok Level:</label>
                <input type="number" id="min_stock_level" name="min_stock_level" value="<%= product.min_stock_level %>" step="any">

                <label for="max_stock_level">Max. Stok Level:</label>
                <input type="number" id="max_stock_level" name="max_stock_level" value="<%= product.max_stock_level || '' %>" step="any">

                <label for="location_default">Lokasi Default:</label>
                <input type="text" id="location_default" name="location_default" value="<%= product.location_default || '' %>">

                <label for="is_active">Status Aktif:</label>
                <input type="checkbox" id="is_active" name="is_active" value="true" <%= product.is_active ? 'checked' : '' %>>

                <button type="submit">Update Produk</button>
            </form>
        <% } else { %>
            <p>Produk tidak ditemukan.</p>
        <% } %>

        <a href="/admin/products" class="back-link">Kembali ke Daftar Produk</a>
    </div>
</body>
</html>
```

Sekarang, Anda seharusnya bisa:

    Login sebagai admin.

    Navigasi ke http://localhost:3000/admin/products untuk melihat daftar produk (yang saat ini mungkin kosong).

    Klik "Tambah Produk Baru" untuk membuka form dan mencoba menambahkan produk.

    Mencoba mengedit atau menghapus setelah ada produk.
### buka `src/app.js`
Cari bagian ini di akhir file app.js Anda:
```
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

Kemudian, ganti seluruh blok console.log tersebut dengan versi yang diperbarui ini:
```
// Jalankan server
app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
    console.log('\n--- URL Penting ---');
    console.log(`1. Halaman Login: http://localhost:${PORT}/login`);
    console.log(`2. Dashboard Admin (Akses setelah Login): http://localhost:${PORT}/dashboard/admin`);
    console.log('\n   --- Manajemen Pengguna (Akses setelah Login sebagai Admin) ---');
    console.log(`   - Buat Pengguna Baru: http://localhost:${PORT}/admin/users/create`);
    console.log(`   - Lihat Daftar Pengguna: http://localhost:${PORT}/admin/users`);
    console.log(`   - Ubah Password Admin: http://localhost:${PORT}/admin/change-password`);

    console.log('\n   --- Manajemen Produk (BARU - Akses setelah Login sebagai Admin) ---');
    console.log(`   - Buat Produk Baru: http://localhost:${PORT}/admin/products/create`);
    console.log(`   - Lihat Daftar Produk: http://localhost:${PORT}/admin/products`);
    console.log('--------------------');
});
```

### buat tabel `location` dan `inventory` stock di mysql
```
mysql -u root -p
```
```
use wms_db;
```

```
CREATE TABLE locations (
    id INT AUTO_INCREMENT PRIMARY KEY,
    location_code VARCHAR(255) UNIQUE NOT NULL, -- Kode unik untuk setiap lokasi (misalnya, A1-R2-S3-B4)
    warehouse_name VARCHAR(255) NOT NULL, -- Nama atau pengenal untuk gudang tertentu
    zone_area VARCHAR(255),                  -- Opsional: Zona atau Area di dalam gudang
    aisle VARCHAR(255),                     -- Opsional: Lorong di dalam zona/area
    rack VARCHAR(255),
    shelf_level VARCHAR(255),
    bin_box_pallet VARCHAR(255),
    location_type ENUM('Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet') NOT NULL, -- misalnya, 'Rack', 'Bin', 'Pallet'
    parent_location_id INT,               -- Foreign key untuk merepresentasikan hirarki (misalnya, Bin termasuk dalam Shelf)
    capacity INT,                          -- Opsional: Kapasitas penyimpanan maksimum lokasi ini
    description TEXT,                      -- Opsional: Detail tambahan tentang lokasi
    is_active BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (parent_location_id) REFERENCES locations(id)
);
```

```
CREATE TABLE inventory_stock (
    id INT AUTO_INCREMENT PRIMARY KEY,
    product_id INT NOT NULL,
    location_id INT NOT NULL,
    quantity INT NOT NULL,
    batch_number VARCHAR(255),             -- Opsional: Nomor batch atau lot
    expiry_date DATE,                       -- Opsional: Tanggal kedaluwarsa untuk item di lokasi ini
    FOREIGN KEY (product_id) REFERENCES products(id),
    FOREIGN KEY (location_id) REFERENCES locations(id)
);
```

###Penjelasan:

###Tabel locations:

    id: ID unik untuk setiap lokasi.

    location_code: Kode unik yang mengidentifikasi lokasi secara spesifik (misalnya, JKT-GUD01-A1-R2-S3-B4 untuk Gudang 01 di Jakarta, Lorong A1, Rak 2, Tingkat 3, Box 4). Ini memungkinkan Anda untuk memiliki lokasi yang sama di gudang yang berbeda.

    warehouse_name: Nama atau pengenal untuk gudang (misalnya, "Gudang Jakarta", "Gudang Surabaya").

    zone_area, aisle, rack, shelf_level, bin_box_pallet: Kolom-kolom ini memungkinkan Anda untuk membuat struktur lokasi yang hierarkis.

    location_type: Menentukan jenis lokasi (Gudang, Zona, Rak, dll.).

    parent_location_id: Menghubungkan lokasi ke lokasi induknya (misalnya, Bin termasuk dalam Shelf).

    capacity: (Opsional) Kapasitas penyimpanan lokasi.

    description: (Opsional) Deskripsi tambahan tentang lokasi.

    is_active: Menandakan apakah lokasi aktif atau tidak.

###Tabel inventory_stock:

    id: ID unik untuk setiap catatan stok.

    product_id: ID produk (dari tabel products).

    location_id: ID lokasi (dari tabel locations).

    quantity: Jumlah item di lokasi tersebut.

    batch_number: (Opsional) Nomor batch atau lot.

    expiry_date: (Opsional) Tanggal kedaluwarsa.

###Dengan struktur ini, Anda dapat melacak lokasi setiap item secara detail, bahkan jika Anda memiliki banyak gudang di tempat yang berbeda. Kolom warehouse_name dan location_code akan menjadi kunci untuk membedakan lokasi di berbagai gudang.

### `wms-app/src/models/Location.js`
```
// src/models/Location.js
const { pool } = require('../config/database');

class Location {
    // Mengambil semua lokasi
    static async getAll() {
        // Mengambil semua lokasi, mengurutkan berdasarkan nama gudang dan kode lokasi
        const [rows] = await pool.query('SELECT * FROM locations ORDER BY warehouse_name ASC, location_code ASC');
        return rows;
    }

    // Mengambil lokasi berdasarkan ID
    static async findById(id) {
        const [rows] = await pool.query('SELECT * FROM locations WHERE id = ?', [id]);
        return rows[0];
    }

    // Mengambil lokasi berdasarkan kode lokasi (penting untuk identifikasi unik)
    static async findByLocationCode(locationCode) {
        const [rows] = await pool.query('SELECT * FROM locations WHERE location_code = ?', [locationCode]);
        return rows[0];
    }

    // Membuat lokasi baru
    static async create({ warehouse_name, location_code, zone_area, aisle, rack, shelf_level, bin_box_pallet, location_type, parent_location_id, capacity, description }) {
        const [result] = await pool.query(
            'INSERT INTO locations (warehouse_name, location_code, zone_area, aisle, rack, shelf_level, bin_box_pallet, location_type, parent_location_id, capacity, description) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)',
            [warehouse_name, location_code, zone_area || null, aisle || null, rack || null, shelf_level || null, bin_box_pallet || null, location_type, parent_location_id || null, capacity || null, description || null]
        );
        return result.insertId;
    }

    // Memperbarui lokasi berdasarkan ID
    static async update(id, { warehouse_name, location_code, zone_area, aisle, rack, shelf_level, bin_box_pallet, location_type, parent_location_id, capacity, description, is_active }) {
        const [result] = await pool.query(
            'UPDATE locations SET warehouse_name = ?, location_code = ?, zone_area = ?, aisle = ?, rack = ?, shelf_level = ?, bin_box_pallet = ?, location_type = ?, parent_location_id = ?, capacity = ?, description = ?, is_active = ? WHERE id = ?',
            [warehouse_name, location_code, zone_area || null, aisle || null, rack || null, shelf_level || null, bin_box_pallet || null, location_type, parent_location_id || null, capacity || null, description || null, is_active, id]
        );
        return result.affectedRows;
    }

    // Menghapus lokasi berdasarkan ID
    static async delete(id) {
        const [result] = await pool.query('DELETE FROM locations WHERE id = ?', [id]);
        return result.affectedRows;
    }

    // Dapatkan lokasi parent (opsional, untuk tampilan hirarki)
    static async getParent(id) {
        const [rows] = await pool.query(`
            SELECT p.* FROM locations AS l
            JOIN locations AS p ON l.parent_location_id = p.id
            WHERE l.id = ?
        `, [id]);
        return rows[0];
    }

    // Dapatkan child locations (opsional, untuk tampilan hirarki)
    static async getChildren(parentId) {
        const [rows] = await pool.query('SELECT * FROM locations WHERE parent_location_id = ?', [parentId]);
        return rows;
    }

    // Metode untuk mendapatkan semua lokasi yang merupakan 'Warehouse'
    static async getAllWarehouses() {
        const [rows] = await pool.query("SELECT * FROM locations WHERE location_type = 'Warehouse' ORDER BY warehouse_name ASC");
        return rows;
    }
}

module.exports = Location;
```

Penjelasan Kode Location.js:

    Mengimport pool dari konfigurasi database.

    Menyediakan metode statis untuk operasi CRUD dasar (getAll, findById, findByLocationCode, create, update, delete).

    Menambahkan metode pembantu untuk hierarki (getParent, getChildren) yang akan sangat berguna saat Anda membangun UI untuk lokasi.

    Menambahkan getAllWarehouses() untuk memudahkan pengambilan daftar gudang utama.

### `wms-app/src/models/InventoryStock.js`
```
// src/models/InventoryStock.js
const { pool } = require('../config/database');

class InventoryStock {
    // Menambahkan stok baru ke lokasi tertentu
    static async create({ product_id, location_id, quantity, batch_number, expiry_date }) {
        const [result] = await pool.query(
            'INSERT INTO inventory_stock (product_id, location_id, quantity, batch_number, expiry_date) VALUES (?, ?, ?, ?, ?)',
            [product_id, location_id, quantity, batch_number || null, expiry_date || null]
        );
        return result.insertId;
    }

    // Mengambil semua catatan stok
    static async getAll() {
        const [rows] = await pool.query(`
            SELECT
                ist.id,
                ist.quantity,
                ist.batch_number,
                ist.expiry_date,
                p.sku,
                p.name AS product_name,
                p.unit_of_measure,
                l.location_code,
                l.warehouse_name,
                l.location_type,
                l.description AS location_description
            FROM inventory_stock ist
            JOIN products p ON ist.product_id = p.id
            JOIN locations l ON ist.location_id = l.id
            ORDER BY p.name ASC, l.location_code ASC
        `);
        return rows;
    }

    // Mengambil catatan stok berdasarkan ID
    static async findById(id) {
        const [rows] = await pool.query(`
            SELECT
                ist.id,
                ist.product_id,
                ist.location_id,
                ist.quantity,
                ist.batch_number,
                ist.expiry_date,
                p.sku,
                p.name AS product_name,
                p.unit_of_measure,
                l.location_code,
                l.warehouse_name,
                l.location_type,
                l.description AS location_description
            FROM inventory_stock ist
            JOIN products p ON ist.product_id = p.id
            JOIN locations l ON ist.location_id = l.id
            WHERE ist.id = ?
        `, [id]);
        return rows[0];
    }

    // Mengambil stok untuk produk tertentu di lokasi tertentu
    static async findByProductAndLocation(product_id, location_id) {
        const [rows] = await pool.query(
            'SELECT * FROM inventory_stock WHERE product_id = ? AND location_id = ?',
            [product_id, location_id]
        );
        return rows; // Mungkin ada beberapa batch untuk produk yang sama di lokasi yang sama
    }

    // Memperbarui kuantitas stok di lokasi tertentu
    static async updateQuantity(id, newQuantity) {
        const [result] = await pool.query(
            'UPDATE inventory_stock SET quantity = ? WHERE id = ?',
            [newQuantity, id]
        );
        return result.affectedRows;
    }

    // Menghapus catatan stok
    static async delete(id) {
        const [result] = await pool.query('DELETE FROM inventory_stock WHERE id = ?', [id]);
        return result.affectedRows;
    }

    // Dapatkan total stok untuk sebuah produk di semua lokasi
    static async getTotalStockByProductId(productId) {
        const [rows] = await pool.query(
            'SELECT SUM(quantity) as total_stock FROM inventory_stock WHERE product_id = ?',
            [productId]
        );
        return rows[0].total_stock || 0;
    }

    // Mendapatkan semua stok untuk sebuah produk, diurutkan berdasarkan lokasi
    static async getStockByProductId(productId) {
        const [rows] = await pool.query(`
            SELECT
                ist.id,
                ist.quantity,
                ist.batch_number,
                ist.expiry_date,
                l.location_code,
                l.warehouse_name,
                l.location_type,
                l.description AS location_description
            FROM inventory_stock ist
            JOIN locations l ON ist.location_id = l.id
            WHERE ist.product_id = ?
            ORDER BY l.warehouse_name, l.location_code
        `, [productId]);
        return rows;
    }
}

module.exports = InventoryStock;
```

Penjelasan Kode InventoryStock.js:

    create: Menambahkan entri stok baru. Penting untuk dicatat bahwa batch_number dan expiry_date adalah opsional.

    getAll: Mengambil semua catatan stok dan melakukan JOIN dengan tabel products dan locations untuk mendapatkan detail nama produk, SKU, nama lokasi, dll. Ini akan sangat berguna untuk tampilan daftar stok.

    findById: Mengambil detail satu catatan stok.

    findByProductAndLocation: Menemukan semua catatan stok untuk produk tertentu di lokasi tertentu. Ini penting karena satu produk bisa memiliki beberapa batch di lokasi yang sama.

    updateQuantity: Mengubah jumlah stok untuk catatan inventory_stock tertentu.

    delete: Menghapus catatan stok.

    getTotalStockByProductId: Fungsi agregasi untuk mendapatkan total stok sebuah produk di seluruh gudang. Ini akan digunakan untuk memperbarui kolom current_stock_quantity di tabel products.

    getStockByProductId: Mendapatkan semua catatan stok untuk satu produk tertentu, menampilkan lokasi dan detailnya.


### Langkah selanjutnya adalah membuat Controller untuk manajemen lokasi. Ini akan mengelola logika bisnis untuk operasi CRUD pada lokasi gudang Anda.
### `wms-app/src/controllers/locationController.js`
```
// src/controllers/locationController.js
const Location = require('../models/Location'); // Import model Location

// Fungsi untuk menampilkan daftar lokasi
exports.listLocations = async (req, res) => {
    try {
        const locations = await Location.getAll(); // Ambil semua lokasi dari database
        const message = req.query.message || null;
        const error = req.query.error || null;

        res.render('admin/locations/list', { // Render view list.ejs di folder admin/locations
            locations,
            user: req.user,
            message: message,
            error: error
        });
    } catch (err) {
        console.error('Error fetching locations:', err);
        res.render('error', { message: 'Gagal mengambil data lokasi.' });
    }
};

// Fungsi untuk menampilkan form tambah lokasi baru
exports.showCreateLocationForm = async (req, res) => {
    try {
        const parentLocations = await Location.getAll(); // Ambil semua lokasi untuk opsi parent
        const locationTypes = ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'];

        res.render('admin/locations/create', {
            user: req.user,
            parentLocations: parentLocations,
            locationTypes: locationTypes,
            error: null
        });
    } catch (err) {
        console.error('Error showing create location form:', err);
        res.render('error', { message: 'Gagal menampilkan form penambahan lokasi.' });
    }
};

// Fungsi untuk memproses penambahan lokasi baru
exports.createLocation = async (req, res) => {
    const { warehouse_name, location_code, zone_area, aisle, rack, shelf_level, bin_box_pallet, location_type, parent_location_id, capacity, description } = req.body;

    // Validasi input dasar
    if (!warehouse_name || !location_code || !location_type) {
        // Ambil lagi data parentLocations dan locationTypes untuk di-render ulang jika ada error
        const parentLocations = await Location.getAll();
        const locationTypes = ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'];
        return res.render('admin/locations/create', {
            user: req.user,
            parentLocations: parentLocations,
            locationTypes: locationTypes,
            error: 'Nama Gudang, Kode Lokasi, dan Tipe Lokasi harus diisi.'
        });
    }

    try {
        // Cek apakah kode lokasi sudah ada
        const existingLocation = await Location.findByLocationCode(location_code);
        if (existingLocation) {
            const parentLocations = await Location.getAll();
            const locationTypes = ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'];
            return res.render('admin/locations/create', {
                user: req.user,
                parentLocations: parentLocations,
                locationTypes: locationTypes,
                error: `Kode Lokasi "${location_code}" sudah ada. Gunakan kode lain.`
            });
        }

        await Location.create({
            warehouse_name,
            location_code,
            zone_area,
            aisle,
            rack,
            shelf_level,
            bin_box_pallet,
            location_type,
            parent_location_id: parent_location_id === '' ? null : parseInt(parent_location_id), // Konversi ke int, set null jika kosong
            capacity: capacity === '' ? null : parseInt(capacity), // Konversi ke int, set null jika kosong
            description
        });

        res.redirect('/admin/locations?message=Lokasi berhasil ditambahkan!');
    } catch (err) {
        console.error('Error creating location:', err);
        const parentLocations = await Location.getAll();
        const locationTypes = ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'];
        res.render('admin/locations/create', {
            user: req.user,
            parentLocations: parentLocations,
            locationTypes: locationTypes,
            error: 'Gagal menambahkan lokasi. Terjadi kesalahan server.'
        });
    }
};

// Fungsi untuk menampilkan form edit lokasi
exports.showEditLocationForm = async (req, res) => {
    try {
        const location = await Location.findById(req.params.id);
        if (!location) {
            return res.redirect('/admin/locations?error=Lokasi tidak ditemukan.');
        }
        const parentLocations = await Location.getAll(); // Ambil semua lokasi untuk opsi parent
        const locationTypes = ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'];

        res.render('admin/locations/edit', {
            location,
            user: req.user,
            parentLocations: parentLocations,
            locationTypes: locationTypes,
            error: null
        });
    } catch (err) {
        console.error('Error fetching location for edit:', err);
        res.render('error', { message: 'Gagal mengambil data lokasi untuk diedit.' });
    }
};

// Fungsi untuk memproses pembaruan lokasi
exports.updateLocation = async (req, res) => {
    const { id } = req.params;
    const { warehouse_name, location_code, zone_area, aisle, rack, shelf_level, bin_box_pallet, location_type, parent_location_id, capacity, description, is_active } = req.body;
    const isActiveBoolean = is_active === 'true';

    // Validasi input dasar
    if (!warehouse_name || !location_code || !location_type) {
        return res.redirect(`/admin/locations/${id}/edit?error=Nama Gudang, Kode Lokasi, dan Tipe Lokasi harus diisi.`);
    }

    try {
        // Cek apakah kode lokasi baru sudah digunakan oleh lokasi lain (kecuali lokasi ini sendiri)
        const existingLocation = await Location.findByLocationCode(location_code);
        if (existingLocation && existingLocation.id !== parseInt(id)) {
            return res.redirect(`/admin/locations/${id}/edit?error=Kode Lokasi "${location_code}" sudah digunakan oleh lokasi lain.`);
        }

        await Location.update(id, {
            warehouse_name,
            location_code,
            zone_area: zone_area || null,
            aisle: aisle || null,
            rack: rack || null,
            shelf_level: shelf_level || null,
            bin_box_pallet: bin_box_pallet || null,
            location_type,
            parent_location_id: parent_location_id === '' ? null : parseInt(parent_location_id),
            capacity: capacity === '' ? null : parseInt(capacity),
            description: description || null,
            is_active: isActiveBoolean
        });

        res.redirect('/admin/locations?message=Lokasi berhasil diperbarui!');
    } catch (err) {
        console.error('Error updating location:', err);
        res.redirect(`/admin/locations/${id}/edit?error=Gagal memperbarui lokasi. Terjadi kesalahan server.`);
    }
};

// Fungsi untuk memproses penghapusan lokasi
exports.deleteLocation = async (req, res) => {
    const { id } = req.params;
    try {
        // TODO: Tambahkan validasi di sini untuk memastikan tidak ada stok yang terkait dengan lokasi ini
        // Atau, pastikan tidak ada lokasi child yang terhubung ke lokasi ini
        const location = await Location.findById(id);
        if (!location) {
            return res.redirect('/admin/locations?error=Lokasi tidak ditemukan.');
        }

        // Contoh validasi: Cek apakah ada child locations
        const children = await Location.getChildren(id);
        if (children && children.length > 0) {
            return res.redirect('/admin/locations?error=Tidak dapat menghapus lokasi ini karena memiliki lokasi anak.');
        }

        // Contoh validasi: Cek apakah ada inventory_stock di lokasi ini
        const [stockCount] = await pool.query('SELECT COUNT(*) as count FROM inventory_stock WHERE location_id = ?', [id]);
        if (stockCount[0].count > 0) {
            return res.redirect('/admin/locations?error=Tidak dapat menghapus lokasi ini karena masih ada stok produk di dalamnya.');
        }


        await Location.delete(id);
        res.redirect('/admin/locations?message=Lokasi berhasil dihapus!');
    } catch (err) {
        console.error('Error deleting location:', err);
        res.redirect('/admin/locations?error=Gagal menghapus lokasi. Terjadi kesalahan server.');
    }
};
```

Penjelasan Kode locationController.js:

    Import Model Location: Digunakan untuk berinteraksi dengan tabel locations.

    listLocations: Mengambil semua lokasi dari database dan merender view admin/locations/list.ejs.

    showCreateLocationForm: Merender view admin/locations/create.ejs. Pentingnya di sini adalah kita juga mengambil semua lokasi yang ada (parentLocations) dan tipe lokasi (locationTypes) untuk diisi di dropdown form, memungkinkan pembentukan hierarki.

    createLocation: Memproses data dari form penambahan lokasi. Melakukan validasi (warehouse_name, location_code, location_type harus diisi) dan memeriksa duplikasi location_code. Menggunakan || null untuk kolom opsional agar disimpan sebagai NULL di database jika kosong.

    showEditLocationForm: Mengambil data lokasi berdasarkan ID untuk mengisi form edit.

    updateLocation: Memproses pembaruan data lokasi, termasuk validasi duplikasi location_code (untuk lokasi lain).

    deleteLocation: Menghapus lokasi. Penting: Saya telah menambahkan validasi awal untuk mencegah penghapusan lokasi jika ada lokasi anak (children) atau jika masih ada stok yang terkait dengan lokasi tersebut. Ini penting untuk menjaga integritas data Anda.

### Anda perlu mengintegrasikan controller lokasi baru ini ke dalam sistem routing admin Anda, sehingga URL seperti /admin/locations dapat diakses dan diatur oleh controller tersebut.

Tambahkan import locationController di bagian atas file, di samping userController dan productController.

Tambahkan rute-rute baru untuk lokasi di bawah rute produk yang sudah ada. Pastikan rute-rute ini tetap berada di dalam blok router yang dilindungi oleh authenticateToken dan authorizeRoles('admin').

### `wms-app/src/routes/adminRoutes.js`
```
// src/routes/adminRoutes.js
const express = require('express');
const router = express.Router();
const { authenticateToken, authorizeRoles } = require('../middleware/authMiddleware');
const userController = require('../controllers/userController');
const productController = require('../controllers/productController');
const locationController = require('../controllers/locationController'); // <-- TAMBAHKAN INI

// Middleware untuk semua rute admin: memastikan user terautentikasi dan memiliki role 'admin'
router.use(authenticateToken);
router.use(authorizeRoles('admin'));

// --- Rute untuk Manajemen Pengguna (User Management) ---
router.get('/users', userController.listUsers);
router.get('/users/create', userController.showCreateUserForm);
router.post('/users/create', userController.createUser);
router.get('/users/:id/edit', userController.showEditUserForm);
router.post('/users/:id/edit', userController.updateUser);
router.post('/users/:id/delete', userController.deleteUser);

// --- Rute untuk Ganti Password Admin ---
router.get('/change-password', userController.showChangePasswordForm);
router.post('/change-password', userController.changePassword);

// --- Rute untuk Manajemen Produk (Product Management) ---
router.get('/products', productController.listProducts);
router.get('/products/create', productController.showCreateProductForm);
router.post('/products/create', productController.createProduct);
router.get('/products/:id/edit', productController.showEditProductForm);
router.post('/products/:id/edit', productController.updateProduct);
router.post('/products/:id/delete', productController.deleteProduct);

// --- Rute BARU untuk Manajemen Lokasi (Location Management) ---
// Tampilkan daftar lokasi
router.get('/locations', locationController.listLocations);
// Tampilkan form tambah lokasi
router.get('/locations/create', locationController.showCreateLocationForm);
// Proses tambah lokasi
router.post('/locations/create', locationController.createLocation);
// Tampilkan form edit lokasi
router.get('/locations/:id/edit', locationController.showEditLocationForm);
// Proses update lokasi
router.post('/locations/:id/edit', locationController.updateLocation);
// Proses hapus lokasi
router.post('/locations/:id/delete', locationController.deleteLocation);


module.exports = router;
```

### `wms-app/views/admin/locations/list.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Manajemen Lokasi - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 90%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; }
        .header-controls { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
        .header-controls a { background-color: #007bff; color: white; padding: 10px 15px; text-decoration: none; border-radius: 5px; }
        .header-controls a:hover { background-color: #0056b3; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        table { width: 100%; border-collapse: collapse; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
        .actions a, .actions button {
            padding: 5px 10px;
            text-decoration: none;
            border-radius: 3px;
            margin-right: 5px;
            font-size: 0.9em;
        }
        .actions a.edit-btn { background-color: #ffc107; color: black; }
        .actions a.edit-btn:hover { background-color: #e0a800; }
        .actions button.delete-btn { background-color: #dc3545; color: white; border: none; cursor: pointer; }
        .actions button.delete-btn:hover { background-color: #c82333; }
        .status-active { color: green; font-weight: bold; }
        .status-inactive { color: red; font-weight: bold; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/admin/locations">Manajemen Lokasi</a>
        <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <div class="header-controls">
            <h1>Daftar Lokasi Gudang</h1>
            <a href="/admin/locations/create">Tambah Lokasi Baru</a>
        </div>

        <% if (message) { %>
            <div class="messages message-success"><%= message %></div>
        <% } %>
        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <table>
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Nama Gudang</th>
                    <th>Kode Lokasi</th>
                    <th>Tipe</th>
                    <th>Parent</th>
                    <th>Kapasitas</th>
                    <th>Deskripsi</th>
                    <th>Status</th>
                    <th>Aksi</th>
                </tr>
            </thead>
            <tbody>
                <% if (locations && locations.length > 0) { %>
                    <% locations.forEach(location => { %>
                        <tr>
                            <td><%= location.id %></td>
                            <td><%= location.warehouse_name %></td>
                            <td><%= location.location_code %></td>
                            <td><%= location.location_type %></td>
                            <td>
                                <% 
                                    const parent = locations.find(loc => loc.id === location.parent_location_id);
                                    if (parent) {
                                        %><%= parent.location_code %><%
                                    } else {
                                        %>-<%
                                    }
                                %>
                            </td>
                            <td><%= location.capacity || '-' %></td>
                            <td><%= location.description || '-' %></td>
                            <td><span class="<%= location.is_active ? 'status-active' : 'status-inactive' %>">
                                <%= location.is_active ? 'Aktif' : 'Nonaktif' %>
                            </span></td>
                            <td class="actions">
                                <a href="/admin/locations/<%= location.id %>/edit" class="edit-btn">Edit</a>
                                <form action="/admin/locations/<%= location.id %>/delete" method="POST" style="display:inline;" onsubmit="return confirm('Anda yakin ingin menghapus lokasi ini? Ini akan gagal jika ada stok atau lokasi anak di dalamnya.');">
                                    <button type="submit" class="delete-btn">Hapus</button>
                                </form>
                            </td>
                        </tr>
                    <% }); %>
                <% } else { %>
                    <tr>
                        <td colspan="9">Belum ada lokasi gudang yang terdaftar.</td>
                    </tr>
                <% } %>
            </tbody>
        </table>
    </div>
</body>
</html>
```

### `wms-app/views/admin/locations/create.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Tambah Lokasi Baru - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 80%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; margin-bottom: 20px; }
        form { display: grid; grid-template-columns: 1fr 2fr; gap: 15px; align-items: center; }
        label { font-weight: bold; }
        input[type="text"], input[type="number"], textarea, select {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        textarea { resize: vertical; min-height: 80px; }
        button {
            grid-column: 1 / 3;
            padding: 10px 15px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover { background-color: #0056b3; }
        .back-link { display: block; margin-top: 20px; text-decoration: none; color: #007bff; }
        .back-link:hover { text-decoration: underline; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/admin/locations">Manajemen Lokasi</a>
        <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <h1>Tambah Lokasi Gudang Baru</h1>

        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <form action="/admin/locations/create" method="POST">
            <label for="warehouse_name">Nama Gudang:</label>
            <input type="text" id="warehouse_name" name="warehouse_name" required placeholder="Contoh: Gudang Jakarta Pusat">

            <label for="location_code">Kode Lokasi:</label>
            <input type="text" id="location_code" name="location_code" required placeholder="Contoh: JKT-GUD01-A1-R2-S3-B4">

            <label for="location_type">Tipe Lokasi:</label>
            <select id="location_type" name="location_type" required>
                <option value="">-- Pilih Tipe Lokasi --</option>
                <% locationTypes.forEach(type => { %>
                    <option value="<%= type %>"><%= type %></option>
                <% }); %>
            </select>

            <label for="parent_location_id">Lokasi Induk (Parent):</label>
            <select id="parent_location_id" name="parent_location_id">
                <option value="">-- Tidak Ada (Top Level) --</option>
                <% parentLocations.forEach(loc => { %>
                    <option value="<%= loc.id %>"><%= loc.warehouse_name %> - <%= loc.location_code %> (<%= loc.location_type %>)</option>
                <% }); %>
            </select>

            <label for="zone_area">Zona/Area:</label>
            <input type="text" id="zone_area" name="zone_area">

            <label for="aisle">Lorong (Aisle):</label>
            <input type="text" id="aisle" name="aisle">

            <label for="rack">Rak (Rack):</label>
            <input type="text" id="rack" name="rack">

            <label for="shelf_level">Tingkat/Ambalan (Shelf/Level):</label>
            <input type="text" id="shelf_level" name="shelf_level">

            <label for="bin_box_pallet">Bin/Box/Palet:</label>
            <input type="text" id="bin_box_pallet" name="bin_box_pallet">

            <label for="capacity">Kapasitas (jumlah/volume):</label>
            <input type="number" id="capacity" name="capacity" min="0">

            <label for="description">Deskripsi:</label>
            <textarea id="description" name="description"></textarea>

            <button type="submit">Tambah Lokasi</button>
        </form>
        <a href="/admin/locations" class="back-link">Kembali ke Daftar Lokasi</a>
    </div>
</body>
</html>
```

### `wms-app/views/admin/locations/edit.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Edit Lokasi - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 80%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; margin-bottom: 20px; }
        form { display: grid; grid-template-columns: 1fr 2fr; gap: 15px; align-items: center; }
        label { font-weight: bold; }
        input[type="text"], input[type="number"], textarea, select {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        textarea { resize: vertical; min-height: 80px; }
        button {
            grid-column: 1 / 3;
            padding: 10px 15px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover { background-color: #0056b3; }
        .back-link { display: block; margin-top: 20px; text-decoration: none; color: #007bff; }
        .back-link:hover { text-decoration: underline; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/admin/locations">Manajemen Lokasi</a>
        <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <h1>Edit Lokasi Gudang</h1>

        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <% if (location) { %>
            <form action="/admin/locations/<%= location.id %>/edit" method="POST">
                <label for="warehouse_name">Nama Gudang:</label>
                <input type="text" id="warehouse_name" name="warehouse_name" value="<%= location.warehouse_name %>" required>

                <label for="location_code">Kode Lokasi:</label>
                <input type="text" id="location_code" name="location_code" value="<%= location.location_code %>" required>

                <label for="location_type">Tipe Lokasi:</label>
                <select id="location_type" name="location_type" required>
                    <% locationTypes.forEach(type => { %>
                        <option value="<%= type %>" <%= location.location_type === type ? 'selected' : '' %>><%= type %></option>
                    <% }); %>
                </select>

                <label for="parent_location_id">Lokasi Induk (Parent):</label>
                <select id="parent_location_id" name="parent_location_id">
                    <option value="">-- Tidak Ada (Top Level) --</option>
                    <% parentLocations.forEach(loc => { %>
                        <% if (loc.id !== location.id) { %> <option value="<%= loc.id %>" <%= location.parent_location_id === loc.id ? 'selected' : '' %>>
                                <%= loc.warehouse_name %> - <%= loc.location_code %> (<%= loc.location_type %>)
                            </option>
                        <% } %>
                    <% }); %>
                </select>

                <label for="zone_area">Zona/Area:</label>
                <input type="text" id="zone_area" name="zone_area" value="<%= location.zone_area || '' %>">

                <label for="aisle">Lorong (Aisle):</label>
                <input type="text" id="aisle" name="aisle" value="<%= location.aisle || '' %>">

                <label for="rack">Rak (Rack):</label>
                <input type="text" id="rack" name="rack" value="<%= location.rack || '' %>">

                <label for="shelf_level">Tingkat/Ambalan (Shelf/Level):</label>
                <input type="text" id="shelf_level" name="shelf_level" value="<%= location.shelf_level || '' %>">

                <label for="bin_box_pallet">Bin/Box/Palet:</label>
                <input type="text" id="bin_box_pallet" name="bin_box_pallet" value="<%= location.bin_box_pallet || '' %>">

                <label for="capacity">Kapasitas (jumlah/volume):</label>
                <input type="number" id="capacity" name="capacity" value="<%= location.capacity || '' %>" min="0">

                <label for="description">Deskripsi:</label>
                <textarea id="description" name="description"><%= location.description || '' %></textarea>

                <label for="is_active">Status Aktif:</label>
                <input type="checkbox" id="is_active" name="is_active" value="true" <%= location.is_active ? 'checked' : '' %>>

                <button type="submit">Update Lokasi</button>
            </form>
        <% } else { %>
            <p>Lokasi tidak ditemukan.</p>
        <% } %>

        <a href="/admin/locations" class="back-link">Kembali ke Daftar Lokasi</a>
    </div>
</body>
</html>
```

### Sekarang, silakan coba tambahkan beberapa lokasi untuk menguji fungsionalitasnya.

Anda bisa mencoba membuat struktur hierarki seperti yang kita diskusikan sebelumnya:

### Buat Gudang Utama (level paling atas):

    Nama Gudang: Gudang Utama Jakarta

    Kode Lokasi: JKT-GUD01

    Tipe Lokasi: Warehouse

    Lokasi Induk: -- Tidak Ada (Top Level) -- (biarkan kosong)

    Sisa field bisa Anda isi sesuai kebutuhan atau biarkan kosong.

    Klik "Tambah Lokasi".

###Kemudian, tambahkan Zona di bawah Gudang Utama:

    Nama Gudang: Gudang Utama Jakarta (gunakan nama gudang yang sama)

    Kode Lokasi: JKT-GUD01-ZONE-A

    Tipe Lokasi: Zone

    Lokasi Induk: Pilih "JKT-GUD01 (Warehouse)"

    Zona/Area: A

    Sisa field bisa Anda isi.

    Klik "Tambah Lokasi".

###Lanjutkan dengan menambahkan Rak, Shelf, Bin/Box/Pallet di bawah lokasi yang sesuai.

    Misalnya, Rak 1 di bawah JKT-GUD01-ZONE-A:

    Nama Gudang: Gudang Utama Jakarta

    Kode Lokasi: JKT-GUD01-ZONE-A-R1

    Tipe Lokasi: Rack

    Lokasi Induk: Pilih "JKT-GUD01-ZONE-A (Zone)"

    Rak: 1

    Sisa field bisa Anda isi.

### tambah kan url ini ke `wms-app/src/app.js`

```
    console.log('\n   --- Manajemen Lokasi (BARU - Akses setelah Login sebagai Admin) ---'); // <-- TAMBAHKAN BARIS INI
    console.log(`   - Buat Lokasi Baru: http://localhost:${PORT}/admin/locations/create`); // <-- TAMBAHKAN BARIS INI
    console.log(`   - Lihat Daftar Lokasi: http://localhost:${PORT}/admin/locations`); // <-- TAMBAHKAN BARIS INI

```

### `wms-app/src/controllers/inventoryStockController.js`
```
// src/controllers/inventoryStockController.js
const InventoryStock = require('../models/InventoryStock');
const Product = require('../models/Product'); // Perlu untuk update current_stock_quantity
const Location = require('../models/Location'); // Perlu untuk dropdown lokasi

// Fungsi bantu untuk memperbarui total stok di tabel products
async function updateProductCurrentStock(productId) {
    const totalStock = await InventoryStock.getTotalStockByProductId(productId);
    await Product.updateCurrentStock(productId, totalStock);
}

// Fungsi untuk menampilkan daftar stok inventaris
exports.listInventoryStocks = async (req, res) => {
    try {
        const stocks = await InventoryStock.getAll(); // Ambil semua catatan stok
        const message = req.query.message || null;
        const error = req.query.error || null;

        res.render('admin/inventory_stocks/list', { // Render view list.ejs di folder admin/inventory_stocks
            stocks,
            user: req.user,
            message: message,
            error: error
        });
    } catch (err) {
        console.error('Error fetching inventory stocks:', err);
        res.render('error', { message: 'Gagal mengambil data stok inventaris.' });
    }
};

// Fungsi untuk menampilkan form tambah stok baru (barang masuk)
exports.showAddStockForm = async (req, res) => {
    try {
        const products = await Product.getAll(); // Ambil semua produk untuk dropdown
        const locations = await Location.getAll(); // Ambil semua lokasi untuk dropdown

        res.render('admin/inventory_stocks/add', {
            user: req.user,
            products: products,
            locations: locations,
            error: null
        });
    } catch (err) {
        console.error('Error showing add stock form:', err);
        res.render('error', { message: 'Gagal menampilkan form penambahan stok.' });
    }
};

// Fungsi untuk memproses penambahan stok baru
exports.addStock = async (req, res) => {
    const { product_id, location_id, quantity, batch_number, expiry_date } = req.body;

    if (!product_id || !location_id || !quantity || quantity <= 0) {
        const products = await Product.getAll();
        const locations = await Location.getAll();
        return res.render('admin/inventory_stocks/add', {
            user: req.user,
            products: products,
            locations: locations,
            error: 'Produk, Lokasi, dan Kuantitas harus diisi (kuantitas harus lebih dari 0).'
        });
    }

    try {
        // Cek apakah sudah ada stok produk yang sama di lokasi yang sama dengan batch_number yang sama
        // Jika ada, update kuantitas. Jika tidak, buat baru.
        let existingStocks = await InventoryStock.findByProductAndLocation(product_id, location_id);
        
        let foundExistingBatch = false;
        if (existingStocks && existingStocks.length > 0) {
            for (let stock of existingStocks) {
                // Periksa batch_number jika ada
                if (batch_number && stock.batch_number === batch_number) {
                    await InventoryStock.updateQuantity(stock.id, stock.quantity + parseInt(quantity));
                    foundExistingBatch = true;
                    break;
                } 
                // Jika batch_number tidak ada, dan hanya ada satu entry stok untuk produk/lokasi ini tanpa batch_number
                else if (!batch_number && !stock.batch_number && existingStocks.length === 1) {
                    await InventoryStock.updateQuantity(stock.id, stock.quantity + parseInt(quantity));
                    foundExistingBatch = true;
                    break;
                }
            }
        }

        if (!foundExistingBatch) {
            // Jika tidak ditemukan batch yang sama atau belum ada stok di lokasi ini
            await InventoryStock.create({
                product_id: parseInt(product_id),
                location_id: parseInt(location_id),
                quantity: parseInt(quantity),
                batch_number: batch_number || null,
                expiry_date: expiry_date || null
            });
        }
        
        // Update total stok produk di tabel products
        await updateProductCurrentStock(product_id);

        res.redirect('/admin/inventory-stocks?message=Stok berhasil ditambahkan!');
    } catch (err) {
        console.error('Error adding stock:', err);
        const products = await Product.getAll();
        const locations = await Location.getAll();
        res.render('admin/inventory_stocks/add', {
            user: req.user,
            products: products,
            locations: locations,
            error: 'Gagal menambahkan stok. Terjadi kesalahan server.'
        });
    }
};

// Fungsi untuk menampilkan form edit stok (untuk update kuantitas atau keluar stok)
exports.showEditStockForm = async (req, res) => {
    try {
        const stockId = req.params.id;
        const stock = await InventoryStock.findById(stockId);
        if (!stock) {
            return res.redirect('/admin/inventory-stocks?error=Catatan stok tidak ditemukan.');
        }

        res.render('admin/inventory_stocks/edit', {
            stock: stock,
            user: req.user,
            error: null
        });
    } catch (err) {
        console.error('Error showing edit stock form:', err);
        res.render('error', { message: 'Gagal menampilkan form edit stok.' });
    }
};

// Fungsi untuk memproses update stok (termasuk keluar stok)
exports.updateStock = async (req, res) => {
    const stockId = req.params.id;
    const { quantity } = req.body; // Kuantitas baru yang diinginkan

    if (!quantity || parseInt(quantity) < 0) { // Kuantitas tidak boleh negatif
        const stock = await InventoryStock.findById(stockId); // Ambil ulang data stok
        return res.render('admin/inventory_stocks/edit', {
            stock: stock,
            user: req.user,
            error: 'Kuantitas harus diisi dan tidak boleh negatif.'
        });
    }

    try {
        const currentStock = await InventoryStock.findById(stockId);
        if (!currentStock) {
            return res.redirect('/admin/inventory-stocks?error=Catatan stok tidak ditemukan.');
        }

        const newQuantity = parseInt(quantity);
        
        // Update kuantitas di tabel inventory_stock
        await InventoryStock.updateQuantity(stockId, newQuantity);

        // Update total stok produk di tabel products
        await updateProductCurrentStock(currentStock.product_id);

        res.redirect('/admin/inventory-stocks?message=Stok berhasil diperbarui!');
    } catch (err) {
        console.error('Error updating stock:', err);
        res.redirect(`/admin/inventory-stocks/${stockId}/edit?error=Gagal memperbarui stok. Terjadi kesalahan server.`);
    }
};

// Fungsi untuk memproses penghapusan catatan stok
exports.deleteStock = async (req, res) => {
    const { id } = req.params;
    try {
        const stockToDelete = await InventoryStock.findById(id);
        if (!stockToDelete) {
            return res.redirect('/admin/inventory-stocks?error=Catatan stok tidak ditemukan.');
        }

        await InventoryStock.delete(id);
        
        // Update total stok produk di tabel products setelah penghapusan
        await updateProductCurrentStock(stockToDelete.product_id);

        res.redirect('/admin/inventory-stocks?message=Catatan stok berhasil dihapus!');
    } catch (err) {
        console.error('Error deleting stock:', err);
        res.redirect('/admin/inventory-stocks?error=Gagal menghapus catatan stok. Terjadi kesalahan server.');
    }
};
```

Penjelasan Kode inventoryStockController.js:

    Import Models: Mengimport InventoryStock, Product, dan Location karena kita akan berinteraksi dengan ketiga tabel ini.

    updateProductCurrentStock(productId): Ini adalah fungsi bantu yang sangat penting. Setiap kali ada perubahan pada inventory_stock untuk suatu product_id, fungsi ini akan menghitung ulang total quantity dari semua entri inventory_stock untuk productId tersebut dan memperbarui kolom current_stock_quantity di tabel products.

    listInventoryStocks: Mengambil semua catatan stok inventaris (dengan detail produk dan lokasi) dan merender view admin/inventory_stocks/list.ejs.

    showAddStockForm: Merender view admin/inventory_stocks/add.ejs. Ini mengambil daftar products dan locations untuk diisi dalam dropdown form, memungkinkan pengguna memilih produk dan lokasi.

    addStock: Memproses penambahan stok baru.

    Melakukan validasi input dasar.

    Mencari apakah sudah ada catatan stok untuk produk dan lokasi yang sama, dan, jika berlaku, dengan batch_number yang sama. Jika ditemukan, kuantitas akan diperbarui. Jika tidak, catatan stok baru akan dibuat.

    Memanggil updateProductCurrentStock() setelah operasi stok berhasil.

    showEditStockForm: Merender view admin/inventory_stocks/edit.ejs dengan data stok yang dipilih.

    updateStock: Memproses pembaruan kuantitas stok. Ini bisa digunakan untuk menambah atau mengurangi stok (misalnya, jika Anda mengurangi kuantitas, itu bisa diartikan sebagai barang keluar).

    Memanggil updateProductCurrentStock() setelah operasi stok berhasil.

    deleteStock: Menghapus catatan stok dari inventory_stock.

    Memanggil updateProductCurrentStock() setelah operasi stok berhasil.

### `wms-app/src/routes/adminRoutes.js`
```
// src/routes/adminRoutes.js
const express = require('express');
const router = express.Router();
const { authenticateToken, authorizeRoles } = require('../middleware/authMiddleware');
const userController = require('../controllers/userController');
const productController = require('../controllers/productController');
const locationController = require('../controllers/locationController');
const inventoryStockController = require('../controllers/inventoryStockController'); // <-- TAMBAHKAN INI

// Middleware untuk semua rute admin: memastikan user terautentikasi dan memiliki role 'admin'
router.use(authenticateToken);
router.use(authorizeRoles('admin'));

// --- Rute untuk Manajemen Pengguna (User Management) ---
router.get('/users', userController.listUsers);
router.get('/users/create', userController.showCreateUserForm);
router.post('/users/create', userController.createUser);
router.get('/users/:id/edit', userController.showEditUserForm);
router.post('/users/:id/edit', userController.updateUser);
router.post('/users/:id/delete', userController.deleteUser);

// --- Rute untuk Ganti Password Admin ---
router.get('/change-password', userController.showChangePasswordForm);
router.post('/change-password', userController.changePassword);

// --- Rute untuk Manajemen Produk (Product Management) ---
router.get('/products', productController.listProducts);
router.get('/products/create', productController.showCreateProductForm);
router.post('/products/create', productController.createProduct);
router.get('/products/:id/edit', productController.showEditProductForm);
router.post('/products/:id/edit', productController.updateProduct);
router.post('/products/:id/delete', productController.deleteProduct);

// --- Rute untuk Manajemen Lokasi (Location Management) ---
router.get('/locations', locationController.listLocations);
router.get('/locations/create', locationController.showCreateLocationForm);
router.post('/locations/create', locationController.createLocation);
router.get('/locations/:id/edit', locationController.showEditLocationForm);
router.post('/locations/:id/edit', locationController.updateLocation);
router.post('/locations/:id/delete', locationController.deleteLocation);

// --- Rute BARU untuk Manajemen Stok Inventaris (Inventory Stock Management) ---
// Tampilkan daftar semua catatan stok
router.get('/inventory-stocks', inventoryStockController.listInventoryStocks);
// Tampilkan form tambah stok baru (barang masuk)
router.get('/inventory-stocks/add', inventoryStockController.showAddStockForm);
// Proses penambahan stok
router.post('/inventory-stocks/add', inventoryStockController.addStock);
// Tampilkan form edit stok (untuk update kuantitas)
router.get('/inventory-stocks/:id/edit', inventoryStockController.showEditStockForm);
// Proses update stok
router.post('/inventory-stocks/:id/edit', inventoryStockController.updateStock);
// Proses hapus catatan stok
router.post('/inventory-stocks/:id/delete', inventoryStockController.deleteStock);


module.exports = router;
```

### `wms-app/views/admin/inventory_stocks/list.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Manajemen Stok Inventaris - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 90%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; }
        .header-controls { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
        .header-controls a { background-color: #007bff; color: white; padding: 10px 15px; text-decoration: none; border-radius: 5px; }
        .header-controls a:hover { background-color: #0056b3; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-success { background-color: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        table { width: 100%; border-collapse: collapse; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
        .actions a, .actions button {
            padding: 5px 10px;
            text-decoration: none;
            border-radius: 3px;
            margin-right: 5px;
            font-size: 0.9em;
        }
        .actions a.edit-btn { background-color: #ffc107; color: black; }
        .actions a.edit-btn:hover { background-color: #e0a800; }
        .actions button.delete-btn { background-color: #dc3545; color: white; border: none; cursor: pointer; }
        .actions button.delete-btn:hover { background-color: #c82333; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/admin/locations">Manajemen Lokasi</a>
        <a href="/admin/inventory-stocks">Manajemen Stok Inventaris</a> <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <div class="header-controls">
            <h1>Daftar Stok Inventaris</h1>
            <a href="/admin/inventory-stocks/add">Tambah Stok Baru (Barang Masuk)</a>
        </div>

        <% if (message) { %>
            <div class="messages message-success"><%= message %></div>
        <% } %>
        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <table>
            <thead>
                <tr>
                    <th>ID</th>
                    <th>SKU</th>
                    <th>Nama Produk</th>
                    <th>Lokasi</th>
                    <th>Gudang</th>
                    <th>Tipe Lokasi</th>
                    <th>Kuantitas</th>
                    <th>Batch</th>
                    <th>Kadaluwarsa</th>
                    <th>Aksi</th>
                </tr>
            </thead>
            <tbody>
                <% if (stocks && stocks.length > 0) { %>
                    <% stocks.forEach(stock => { %>
                        <tr>
                            <td><%= stock.id %></td>
                            <td><%= stock.sku %></td>
                            <td><%= stock.product_name %></td>
                            <td><%= stock.location_code %></td>
                            <td><%= stock.warehouse_name %></td>
                            <td><%= stock.location_type %></td>
                            <td><%= stock.quantity %> <%= stock.unit_of_measure %></td>
                            <td><%= stock.batch_number || '-' %></td>
                            <td><%= stock.expiry_date ? new Date(stock.expiry_date).toLocaleDateString('id-ID') : '-' %></td>
                            <td class="actions">
                                <a href="/admin/inventory-stocks/<%= stock.id %>/edit" class="edit-btn">Edit Kuantitas</a>
                                <form action="/admin/inventory-stocks/<%= stock.id %>/delete" method="POST" style="display:inline;" onsubmit="return confirm('Anda yakin ingin menghapus catatan stok ini? Ini akan mengurangi total stok produk.');">
                                    <button type="submit" class="delete-btn">Hapus</button>
                                </form>
                            </td>
                        </tr>
                    <% }); %>
                <% } else { %>
                    <tr>
                        <td colspan="10">Belum ada catatan stok inventaris yang terdaftar.</td>
                    </tr>
                <% } %>
            </tbody>
        </table>
    </div>
</body>
</html>
```

### `wms-app/views/admin/inventory_stocks/add.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Tambah Stok Inventaris - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 80%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; margin-bottom: 20px; }
        form { display: grid; grid-template-columns: 1fr 2fr; gap: 15px; align-items: center; }
        label { font-weight: bold; }
        input[type="text"], input[type="number"], input[type="date"], select {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        button {
            grid-column: 1 / 3;
            padding: 10px 15px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover { background-color: #0056b3; }
        .back-link { display: block; margin-top: 20px; text-decoration: none; color: #007bff; }
        .back-link:hover { text-decoration: underline; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/admin/locations">Manajemen Lokasi</a>
        <a href="/admin/inventory-stocks">Manajemen Stok Inventaris</a> <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <h1>Tambah Stok Inventaris Baru (Barang Masuk)</h1>

        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <form action="/admin/inventory-stocks/add" method="POST">
            <label for="product_id">Pilih Produk:</label>
            <select id="product_id" name="product_id" required>
                <option value="">-- Pilih Produk --</option>
                <% products.forEach(product => { %>
                    <option value="<%= product.id %>"><%= product.name %> (<%= product.sku %>)</option>
                <% }); %>
            </select>

            <label for="location_id">Pilih Lokasi Gudang:</label>
            <select id="location_id" name="location_id" required>
                <option value="">-- Pilih Lokasi --</option>
                <% locations.forEach(location => { %>
                    <option value="<%= location.id %>"><%= location.warehouse_name %> - <%= location.location_code %></option>
                <% }); %>
            </select>

            <label for="quantity">Kuantitas:</label>
            <input type="number" id="quantity" name="quantity" min="1" required>

            <label for="batch_number">Nomor Batch (Opsional):</label>
            <input type="text" id="batch_number" name="batch_number">

            <label for="expiry_date">Tanggal Kadaluwarsa (Opsional):</label>
            <input type="date" id="expiry_date" name="expiry_date">

            <button type="submit">Tambah Stok</button>
        </form>
        <a href="/admin/inventory-stocks" class="back-link">Kembali ke Daftar Stok Inventaris</a>
    </div>
</body>
</html>
```

### `wms-app/views/admin/inventory_stocks/edit.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Edit Stok Inventaris - Admin WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .container { width: 80%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; margin-bottom: 20px; }
        form { display: grid; grid-template-columns: 1fr 2fr; gap: 15px; align-items: center; }
        label { font-weight: bold; }
        input[type="number"] {
            width: calc(100% - 22px);
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        button {
            grid-column: 1 / 3;
            padding: 10px 15px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover { background-color: #0056b3; }
        .back-link { display: block; margin-top: 20px; text-decoration: none; color: #007bff; }
        .back-link:hover { text-decoration: underline; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .messages { margin-bottom: 15px; padding: 10px; border-radius: 5px; }
        .message-error { background-color: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
        .stock-details {
            grid-column: 1 / 3;
            background-color: #e9e9e9;
            padding: 10px;
            border-radius: 5px;
            margin-bottom: 15px;
        }
        .stock-details p { margin: 5px 0; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/admin/locations">Manajemen Lokasi</a>
        <a href="/admin/inventory-stocks">Manajemen Stok Inventaris</a> <a href="/logout" class="logout">Logout</a>
    </div>
    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <h1>Edit Kuantitas Stok Inventaris</h1>

        <% if (error) { %>
            <div class="messages message-error"><%= error %></div>
        <% } %>

        <% if (stock) { %>
            <form action="/admin/inventory-stocks/<%= stock.id %>/edit" method="POST">
                <div class="stock-details">
                    <p><strong>Produk:</strong> <%= stock.product_name %> (<%= stock.sku %>)</p>
                    <p><strong>Lokasi:</strong> <%= stock.location_code %> (<%= stock.warehouse_name %>)</p>
                    <% if (stock.batch_number) { %><p><strong>Batch:</strong> <%= stock.batch_number %></p><% } %>
                    <% if (stock.expiry_date) { %><p><strong>Kadaluwarsa:</strong> <%= new Date(stock.expiry_date).toLocaleDateString('id-ID') %></p><% } %>
                    <p><strong>Kuantitas Saat Ini:</strong> <%= stock.quantity %> <%= stock.unit_of_measure %></p>
                </div>

                <label for="quantity">Kuantitas Baru:</label>
                <input type="number" id="quantity" name="quantity" value="<%= stock.quantity %>" min="0" required>

                <button type="submit">Update Kuantitas Stok</button>
            </form>
        <% } else { %>
            <p>Catatan stok tidak ditemukan.</p>
        <% } %>

        <a href="/admin/inventory-stocks" class="back-link">Kembali ke Daftar Stok Inventaris</a>
    </div>
</body>
</html>
```

### `Tambahkan "Manajemen Stok Inventaris" ke navbar umum di semua file EJS admin (terutama di dashboard/admin.ejs, admin/users/*.ejs, admin/products/*.ejs, admin/locations/*.ejs).`

###w`ms-app/views/dashboard/admin.ejs`
```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Dashboard Admin - WMS</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }
        .navbar { background-color: #333; overflow: hidden; }
        .navbar a { float: left; display: block; color: #f2f2f2; text-align: center; padding: 14px 20px; text-decoration: none; }
        .navbar a:hover { background-color: #ddd; color: black; }
        .navbar .logout { float: right; }
        .container { width: 90%; margin: 20px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; margin-bottom: 10px; }
        .user-info { text-align: right; margin-bottom: 15px; font-size: 0.9em; color: #666; }
        .dashboard-content { margin-top: 20px; border-top: 1px solid #eee; padding-top: 20px; }
        .dashboard-content p { line-height: 1.6; }
        .dashboard-content ul { list-style: none; padding: 0; }
        .dashboard-content ul li { background-color: #e9e9e9; margin-bottom: 10px; padding: 10px 15px; border-radius: 5px; }
        .dashboard-content ul li a { text-decoration: none; color: #007bff; font-weight: bold; }
        .dashboard-content ul li a:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/dashboard/admin">Dashboard Admin</a>
        <a href="/admin/users">Manajemen Pengguna</a>
        <a href="/admin/products">Manajemen Produk</a>
        <a href="/admin/locations">Manajemen Lokasi</a>
        <a href="/admin/inventory-stocks">Manajemen Stok Inventaris</a> <a href="/logout" class="logout">Logout</a>
    </div>

    <div class="container">
        <div class="user-info">
            <% if (user) { %>
                Anda login sebagai: <strong><%= user.full_name %> (<%= user.role %>)</strong>
            <% } %>
        </div>

        <h1>Selamat Datang, Admin <%= user.full_name || user.username %>!</h1>
        <p>Ini adalah halaman dashboard untuk Administrator. Anda memiliki akses penuh ke semua fitur sistem manajemen gudang.</p>

        <div class="dashboard-content">
            <h2>Pintasan Cepat:</h2>
            <ul>
                <li><a href="/admin/users">Kelola Pengguna</a>: Tambah, edit, atau hapus akun pengguna.</li>
                <li><a href="/admin/products">Kelola Produk</a>: Tambah, edit, atau hapus daftar produk/item.</li>
                <li><a href="/admin/locations">Kelola Lokasi Gudang</a>: Atur struktur gudang (gudang, zona, rak, box/pallet).</li>
                <li><a href="/admin/inventory-stocks">Kelola Stok Inventaris</a>: Tambah barang masuk, kurangi barang keluar, atau perbarui kuantitas stok di lokasi spesifik.</li>
                <li><a href="/admin/change-password">Ganti Password Admin</a>: Ubah kata sandi akun admin Anda.</li>
            </ul>
        </div>
    </div>
</body>
</html>
```

Penjelasan Perubahan:

    Navbar Lengkap: Saya telah menambahkan navbar yang konsisten di bagian atas halaman, menyertakan semua modul manajemen yang telah kita buat (Pengguna, Produk, Lokasi, Stok Inventaris), serta link ke Dashboard Admin dan Logout.

    Informasi Pengguna: Menampilkan siapa yang login dan peran mereka.

    Pintasan Cepat: Menambahkan daftar link yang mudah diakses ke setiap modul manajemen, memberikan penjelasan singkat tentang fungsinya.

    Styling Dasar: Menambahkan beberapa style CSS sederhana untuk membuat tata letak lebih rapi dan mudah dibaca, konsisten dengan style yang sudah ada di file EJS lainnya.

### Langkah selanjutnya dan yang paling penting adalah melakukan pengujian menyeluruh terhadap semua fitur yang telah kita bangun.

Silakan ikuti poin-poin pengujian yang telah saya sebutkan sebelumnya:

Pengujian Manajemen Stok Inventaris:

    Navigasi ke "Manajemen Stok Inventaris" (http://localhost:3000/admin/inventory-stocks).

    Klik "Tambah Stok Baru (Barang Masuk)".

    Pilih produk (yang sudah Anda buat sebelumnya).

    Pilih lokasi (dari lokasi yang sudah Anda buat, misalnya "JKT-GUD01-ZONE-A-R1").

    Masukkan kuantitas (misal: 100).

    (Opsional) Tambahkan Batch Number dan Expiry Date.

    Klik "Tambah Stok".

    Setelah berhasil, kembali ke daftar stok inventaris. Periksa apakah stok yang baru ditambahkan terlihat.

    Penting: Pergi ke "Manajemen Produk" (http://localhost:3000/admin/products) dan periksa apakah kolom "Stok Saat Ini" untuk produk yang Anda tambahkan sudah terupdate secara otomatis. Ini adalah validasi kunci untuk logic updateProductCurrentStock.

    Coba Edit Kuantitas stok yang sudah ada (misalnya, kurangi kuantitas untuk mensimulasikan barang keluar, atau tambahkan lagi). Periksa apakah total stok produk di halaman produk juga berubah.
    Coba Hapus catatan stok. Pastikan total stok produk juga berkurang.

Pengujian Skenario Khusus:

    Coba tambahkan stok untuk produk yang sama di lokasi yang sama tetapi dengan Batch Number yang berbeda. Harusnya membuat catatan stok baru.

    Coba tambahkan stok untuk produk yang sama di lokasi yang sama tanpa Batch Number, jika sudah ada catatan stok tanpa Batch Number, kuantitasnya harus digabungkan.

    Coba hapus lokasi yang masih memiliki stok di dalamnya. Anda seharusnya mendapatkan pesan error (sesuai logic di locationController.js yang kita buat).


### Buka file `src/models/Product.js`

Temukan baris ini:
```
// Memperbarui kuantitas stok saat ini (akan disempurnakan nanti dengan pergerakan stok)
static async updateStockQuantity(id, newQuantity) {
    const [result] = await pool.query('UPDATE products SET current_stock_quantity = ? WHERE
 id = ?', [newQuantity, id]);
    return result.affectedRows;
}
```

Ganti nama fungsinya menjadi seperti ini:
```
// src/models/Product.js
const { pool } = require('../config/database');

class Product {
    // ... (metode-metode lain yang sudah ada: getAll, findById, findBySku, create, update, delete) ...

    // Memperbarui kuantitas stok saat ini (akan disempurnakan nanti dengan pergerakan stok)
    static async updateCurrentStock(id, newQuantity) { // <-- UBAH NAMA FUNGSI DI SINI
        const [result] = await pool.query('UPDATE products SET current_stock_quantity = ? WHERE id = ?', [newQuantity, id]);
        return result.affectedRows;
    }
}

module.exports = Product;
```

### sekarang coba msuk login sebagai admin, dan masuk ke Manajemen stok iventaris, lalu edit salah satu kuantitas,, maka total sto halaman kelola produk akan otomatis menampilkan jumlah total,


### B. Modul Proses Operasional (Workflow Automation)

Setelah inventaris dasar, ini adalah tentang mengotomatiskan alur kerja.

    1. Penerimaan Barang (Receiving):

      -Purchase Order (PO) Tracking: Mengaitkan penerimaan barang dengan PO yang sudah ada.

      -Quality Control (QC) Integration: Proses pemeriksaan kualitas saat barang diterima (misalnya, role QC bisa menandai barang diterima sebagai "Lulus QC" atau "Reject").

      -Putaway Suggestions: Sistem memberikan rekomendasi lokasi penyimpanan berdasarkan aturan (misalnya, FIFO, FEFO, lokasi kosong, lokasi ideal untuk item).

    2. Pengambilan Barang (Picking & Packing):

      -Sales Order (SO) Integration: Mengaitkan proses picking dengan SO yang masuk.

      -Picking List Generation: Membuat daftar barang yang perlu diambil dari lokasi tertentu.

      -Picking Strategies: Mendukung strategi picking yang berbeda (misalnya, batch picking, zone picking).

      -Packing List Generation: Membuat daftar barang yang sudah di-pack untuk pengiriman.

      -Labeling: Fitur untuk mencetak label barang atau shipping label.
    3. Produksi (Jika Relevan dengan Bisnis Anda):

      -Bill of Materials (BOM): Mendefinisikan komponen yang dibutuhkan untuk membuat produk jadi.

      -Work Order Management: Melacak work order dan konsumsi bahan baku dari gudang untuk produksi.


##Langkah pertama adalah membuat tabel baru di database kita untuk mengelola Purchase Order (PO) dan Receiving Records. Ini akan menjadi dasar untuk mengaitkan barang yang masuk dengan PO yang relevan.

Tabel yang akan kita buat:

    purchase_orders: Akan menyimpan detail PO seperti nomor PO, tanggal order, pemasok, status, dll.

    purchase_order_items: Akan menyimpan item-item produk yang ada di setiap PO, termasuk kuantitas yang diorder. Ini adalah tabel junction antara PO dan produk.

    receiving_records: Akan mencatat setiap kali ada penerimaan barang fisik di gudang, mengaitkannya dengan purchase_order_items yang diterima.

Penting: Perubahan ini menyebabkan kode CREATE TABLE untuk products, locations, inventory_stock, dan tabel baru purchase_orders, purchase_order_items, receiving_records tidak lagi dipanggil di fungsi connectDB yang sekarang hanya memanggil createUsersTable.

Ini berarti tabel-tabel tersebut (selain users) tidak akan dibuat secara otomatis saat aplikasi dimulai jika mereka belum ada di database Anda.

Untuk melanjutkan dengan Modul Proses Operasional, kita perlu mengembalikan logic pembuatan tabel ke dalam alur inisialisasi database.

Ada dua pendekatan:

    1.Mengembalikan semua CREATE TABLE ke connectDB / initializeDatabase (Rekomendasi Cepat): Ini adalah cara termudah untuk melanjutkan dan memastikan semua tabel yang kita butuhkan dibuat.

    2.Membuat Fungsi initializeDatabase Terpisah dan Memanggilnya: Pendekatan yang lebih bersih jika Anda ingin menjaga connectDB hanya untuk koneksi, dan memiliki fungsi terpisah yang bertanggung jawab untuk semua pembuatan/pemeriksaan tabel.

    Saya merekomendasikan pendekatan kedua karena lebih terstruktur dan maintainable.

    Langkah-langkah Perbaikan untuk src/config/database.js:

    Buka file src/config/database.js dengan editor Anda.

    Buat fungsi initializeDatabase baru yang akan berisi semua logic CREATE TABLE.

    Panggil initializeDatabase() dari dalam connectDB()

### `wms-app/sec/config/database.js`

Berikut adalah bagaimana src/config/database.js Anda seharusnya terlihat setelah dimodifikasi:

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

// Fungsi baru untuk menginisialisasi semua tabel
async function initializeDatabase() {
    try {
        // Tabel users
        await pool.query(`
            CREATE TABLE IF NOT EXISTS users (
                id INT AUTO_INCREMENT PRIMARY KEY,
                username VARCHAR(50) UNIQUE NOT NULL,
                password VARCHAR(255) NOT NULL, -- Di sini saya menggunakan 'password' bukan 'password_hash' untuk konsistensi
                full_name VARCHAR(100) NOT NULL,
                email VARCHAR(100) UNIQUE,
                role ENUM('admin', 'ppic', 'packing', 'mixing', 'milling', 'qc', 'gudang') NOT NULL,
                is_active BOOLEAN DEFAULT TRUE,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
            );
        `);
        console.log('Table `users` checked/created by app.');

        // Tabel products
        await pool.query(`
            CREATE TABLE IF NOT EXISTS products (
                id INT AUTO_INCREMENT PRIMARY KEY,
                sku VARCHAR(255) NOT NULL UNIQUE,
                name VARCHAR(255) NOT NULL,
                description TEXT,
                category VARCHAR(255),
                unit_of_measure VARCHAR(50),
                current_stock_quantity DECIMAL(10, 2) DEFAULT 0.00,
                min_stock_level INT DEFAULT 0,
                max_stock_level INT,
                location_default VARCHAR(255),
                is_active BOOLEAN DEFAULT TRUE,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
            );
        `);
        console.log('Table `products` checked/created by app.');

        // Tabel locations
        await pool.query(`
            CREATE TABLE IF NOT EXISTS locations (
                id INT AUTO_INCREMENT PRIMARY KEY,
                warehouse_name VARCHAR(255) NOT NULL,
                location_code VARCHAR(255) NOT NULL UNIQUE,
                location_type ENUM('Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet') NOT NULL,
                parent_location_id INT,
                zone VARCHAR(255),
                aisle VARCHAR(255),
                rack VARCHAR(255),
                shelf VARCHAR(255),
                bin VARCHAR(255),
                capacity DECIMAL(10, 2),
                description TEXT,
                is_active BOOLEAN DEFAULT TRUE,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                FOREIGN KEY (parent_location_id) REFERENCES locations(id) ON DELETE CASCADE
            );
        `);
        console.log('Table `locations` checked/created by app.');

        // Tabel inventory_stock
        await pool.query(`
            CREATE TABLE IF NOT EXISTS inventory_stock (
                id INT AUTO_INCREMENT PRIMARY KEY,
                product_id INT NOT NULL,
                location_id INT NOT NULL,
                quantity DECIMAL(10, 2) NOT NULL,
                batch_number VARCHAR(255),
                expiry_date DATE,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT,
                FOREIGN KEY (location_id) REFERENCES locations(id) ON DELETE RESTRICT,
                UNIQUE (product_id, location_id, batch_number)
            );
        `);
        console.log('Table `inventory_stock` checked/created by app.');

        // --- TABEL BARU UNTUK PROSES PENERIMAAN BARANG (RECEIVING) ---
        await pool.query(`
            CREATE TABLE IF NOT EXISTS purchase_orders (
                id INT AUTO_INCREMENT PRIMARY KEY,
                po_number VARCHAR(255) NOT NULL UNIQUE,
                order_date DATE NOT NULL,
                supplier_name VARCHAR(255) NOT NULL,
                status ENUM('Pending', 'Issued', 'Partially Received', 'Received', 'Cancelled') DEFAULT 'Issued',
                expected_delivery_date DATE,
                notes TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
            );
        `);
        console.log('Table `purchase_orders` checked/created by app.');

        await pool.query(`
            CREATE TABLE IF NOT EXISTS purchase_order_items (
                id INT AUTO_INCREMENT PRIMARY KEY,
                po_id INT NOT NULL,
                product_id INT NOT NULL,
                ordered_quantity DECIMAL(10, 2) NOT NULL,
                received_quantity DECIMAL(10, 2) DEFAULT 0.00,
                unit_of_measure VARCHAR(50),
                notes TEXT,
                FOREIGN KEY (po_id) REFERENCES purchase_orders(id) ON DELETE CASCADE,
                FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE RESTRICT,
                UNIQUE (po_id, product_id)
            );
        `);
        console.log('Table `purchase_order_items` checked/created by app.');

        await pool.query(`
            CREATE TABLE IF NOT EXISTS receiving_records (
                id INT AUTO_INCREMENT PRIMARY KEY,
                po_item_id INT NOT NULL,
                received_quantity DECIMAL(10, 2) NOT NULL,
                received_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                received_by INT,
                batch_number VARCHAR(255),
                expiry_date DATE,
                location_id INT,
                qc_status ENUM('Pending', 'Passed', 'Failed') DEFAULT 'Pending',
                notes TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                FOREIGN KEY (po_item_id) REFERENCES purchase_order_items(id) ON DELETE RESTRICT,
                FOREIGN KEY (received_by) REFERENCES users(id) ON DELETE SET NULL,
                FOREIGN KEY (location_id) REFERENCES locations(id) ON DELETE RESTRICT
            );
        `);
        console.log('Table `receiving_records` checked/created by app.');

    } catch (err) {
        console.error('Error initializing database tables:', err);
        process.exit(1);
    }
}

async function connectDB() {
    try {
        const connection = await pool.getConnection();
        console.log('Connected to MySQL database!');
        connection.release();

        await pool.query(`CREATE DATABASE IF NOT EXISTS ${process.env.DB_NAME}`);
        console.log(`Database '${process.env.DB_NAME}' checked/created.`);

        // Panggil fungsi inisialisasi tabel di sini
        await initializeDatabase();

    } catch (err) {
        console.error('Error connecting to database or initializing tables:', err.message);
        process.exit(1);
    }
}

module.exports = {
    pool,
    connectDB
};
```

Perubahan Penting:

    initializeDatabase() Baru: Semua logic CREATE TABLE IF NOT EXISTS dipindahkan ke fungsi initializeDatabase ini.

    Panggilan di connectDB(): await initializeDatabase(); ditambahkan di connectDB() untuk memastikan semua tabel diperiksa/dibuat saat aplikasi terhubung ke database.

    Perbaikan users table password field: Mengubah password_hash menjadi password agar konsisten dengan logic autentikasi kita yang menggunakan password. Jika Anda sudah memiliki password_hash di database Anda, Anda mungkin perlu melakukan migrasi manual atau mengubah kembali nama kolom di sini.

    role di tabel users: Saya mempertahankan role ENUM yang Anda berikan ('admin', 'ppic', 'packing', 'mixing', 'milling', 'qc', 'gudang').

Setelah Anda memperbarui dan menyimpan file src/config/database.js ini:

    Matikan server Node.js Anda (dengan Ctrl+C).

    Jalankan ulang server Anda dengan node src/app.js (atau nodemon).

    Periksa konsol terminal Anda. Anda seharusnya melihat pesan "Table purchase_orders checked/created by app.", "Table purchase_order_items checked/created by app.", dan "Table receiving_records checked/created by app." (serta tabel lainnya) jika mereka belum ada.

```
mysql -u user -p
```

```
USE wms_db;
```

```
SHOW TABLES;
```

```
DESCRIBE purchase_orders;
```

     jika Semua tabel baru (purchase_order_items, purchase_orders, receiving_records) berhasil dibuat di database wms_db.

    Struktur tabel purchase_orders sudah sesuai dengan definisi SQL yang kita berikan, termasuk kolom po_number sebagai UNIQUE dan status sebagai ENUM.

lanjut ke

### `wms-app/src/models/PurchaseOrder.js`
```
// src/models/PurchaseOrder.js
const { pool } = require('../config/database');

class PurchaseOrder {
    // Mendapatkan semua Purchase Orders
    static async getAll() {
        const [rows] = await pool.query('SELECT * FROM purchase_orders ORDER BY order_date DESC, po_number DESC');
        return rows;
    }

    // Mendapatkan Purchase Order berdasarkan ID
    static async findById(id) {
        const [rows] = await pool.query('SELECT * FROM purchase_orders WHERE id = ?', [id]);
        return rows[0];
    }

    // Mendapatkan Purchase Order berdasarkan PO Number
    static async findByPoNumber(poNumber) {
        const [rows] = await pool.query('SELECT * FROM purchase_orders WHERE po_number = ?', [poNumber]);
        return rows[0];
    }

    // Membuat Purchase Order baru
    static async create({ po_number, order_date, supplier_name, expected_delivery_date, notes }) {
        const [result] = await pool.query(
            'INSERT INTO purchase_orders (po_number, order_date, supplier_name, expected_delivery_date, notes) VALUES (?, ?, ?, ?, ?)',
            [po_number, order_date, supplier_name, expected_delivery_date || null, notes || null]
        );
        return result.insertId;
    }

    // Memperbarui Purchase Order
    static async update(id, { po_number, order_date, supplier_name, status, expected_delivery_date, notes }) {
        const [result] = await pool.query(
            'UPDATE purchase_orders SET po_number = ?, order_date = ?, supplier_name = ?, status = ?, expected_delivery_date = ?, notes = ? WHERE id = ?',
            [po_number, order_date, supplier_name, status, expected_delivery_date || null, notes || null, id]
        );
        return result.affectedRows;
    }

    // Memperbarui status Purchase Order
    static async updateStatus(id, newStatus) {
        const [result] = await pool.query(
            'UPDATE purchase_orders SET status = ? WHERE id = ?',
            [newStatus, id]
        );
        return result.affectedRows;
    }

    // Menghapus Purchase Order
    static async delete(id) {
        const [result] = await pool.query('DELETE FROM purchase_orders WHERE id = ?', [id]);
        return result.affectedRows;
    }
}

module.exports = PurchaseOrder;
```

Penjelasan Model PurchaseOrder:

    getAll(): Mengambil semua PO.

    findById(): Mencari PO berdasarkan ID.

    findByPoNumber(): Mencari PO berdasarkan nomor PO unik.

    create(): Menambahkan PO baru ke database.

    update(): Memperbarui detail PO yang sudah ada.

    updateStatus(): Fungsi khusus untuk memperbarui status PO (akan berguna saat barang diterima).

    delete(): Menghapus PO.


### ``wms-app/src/models/PurchaseOrderItem.js`
```
// src/models/PurchaseOrderItem.js
const { pool } = require('../config/database');

class PurchaseOrderItem {
    // Mendapatkan semua item PO
    static async getAll() {
        const [rows] = await pool.query(`
            SELECT
                poi.id,
                poi.po_id,
                poi.product_id,
                poi.ordered_quantity,
                poi.received_quantity,
                poi.unit_of_measure,
                p.sku,
                p.name AS product_name,
                po.po_number,
                po.supplier_name
            FROM purchase_order_items poi
            JOIN products p ON poi.product_id = p.id
            JOIN purchase_orders po ON poi.po_id = po.id
            ORDER BY po.po_number DESC, p.name ASC
        `);
        return rows;
    }

    // Mendapatkan item PO berdasarkan ID
    static async findById(id) {
        const [rows] = await pool.query(`
            SELECT
                poi.id,
                poi.po_id,
                poi.product_id,
                poi.ordered_quantity,
                poi.received_quantity,
                poi.unit_of_measure,
                p.sku,
                p.name AS product_name,
                po.po_number,
                po.supplier_name
            FROM purchase_order_items poi
            JOIN products p ON poi.product_id = p.id
            JOIN purchase_orders po ON poi.po_id = po.id
            WHERE poi.id = ?
        `, [id]);
        return rows[0];
    }

    // Mendapatkan semua item untuk PO tertentu
    static async getItemsByPoId(poId) {
        const [rows] = await pool.query(`
            SELECT
                poi.id,
                poi.po_id,
                poi.product_id,
                poi.ordered_quantity,
                poi.received_quantity,
                poi.unit_of_measure,
                p.sku,
                p.name AS product_name,
                p.description AS product_description,
                p.category AS product_category
            FROM purchase_order_items poi
            JOIN products p ON poi.product_id = p.id
            WHERE poi.po_id = ?
            ORDER BY p.name ASC
        `, [poId]);
        return rows;
    }

    // Membuat item PO baru
    static async create({ po_id, product_id, ordered_quantity, unit_of_measure, notes }) {
        const [result] = await pool.query(
            'INSERT INTO purchase_order_items (po_id, product_id, ordered_quantity, unit_of_measure, notes) VALUES (?, ?, ?, ?, ?)',
            [po_id, product_id, ordered_quantity, unit_of_measure || null, notes || null]
        );
        return result.insertId;
    }

    // Memperbarui item PO (misalnya, jika kuantitas order berubah)
    static async update(id, { ordered_quantity, unit_of_measure, notes }) {
        const [result] = await pool.query(
            'UPDATE purchase_order_items SET ordered_quantity = ?, unit_of_measure = ?, notes = ? WHERE id = ?',
            [ordered_quantity, unit_of_measure || null, notes || null, id]
        );
        return result.affectedRows;
    }

    // Memperbarui kuantitas yang sudah diterima untuk item PO
    static async updateReceivedQuantity(id, receivedQuantityIncrement) {
        const [result] = await pool.query(
            'UPDATE purchase_order_items SET received_quantity = received_quantity + ? WHERE id = ?',
            [receivedQuantityIncrement, id]
        );
        return result.affectedRows;
    }

    // Menghapus item PO
    static async delete(id) {
        const [result] = await pool.query('DELETE FROM purchase_order_items WHERE id = ?', [id]);
        return result.affectedRows;
    }
}

module.exports = PurchaseOrderItem;
```

Penjelasan Model PurchaseOrderItem:

    getAll(): Mengambil semua item PO dari semua PO, dengan join ke tabel products dan purchase_orders untuk mendapatkan detail terkait.

    findById(): Mencari item PO berdasarkan ID-nya, juga dengan join untuk detail.

    getItemsByPoId(poId): Ini adalah fungsi kunci untuk mendapatkan semua item yang terkait dengan satu PO tertentu.

    create(): Menambahkan item produk ke PO.

    update(): Memperbarui detail item PO.
    
    updateReceivedQuantity(id, receivedQuantityIncrement): Ini adalah fungsi yang sangat penting. Ini akan digunakan saat barang diterima untuk menambahkan kuantitas yang baru diterima ke kolom received_quantity item PO ini. Ini membantu melacak berapa banyak dari item PO yang sudah diterima.

    delete(): Menghapus item dari PO.

### `wms-app/src/models/ReceivingRecord.js`
```
// src/models/ReceivingRecord.js
const { pool } = require('../config/database');

class ReceivingRecord {
    // Mendapatkan semua catatan penerimaan
    static async getAll() {
        const [rows] = await pool.query(`
            SELECT
                rr.id,
                rr.po_item_id,
                rr.received_quantity,
                rr.received_date,
                rr.batch_number,
                rr.expiry_date,
                rr.qc_status,
                rr.notes,
                p.sku AS product_sku,
                p.name AS product_name,
                po.po_number,
                l.location_code,
                u.full_name AS received_by_user
            FROM receiving_records rr
            JOIN purchase_order_items poi ON rr.po_item_id = poi.id
            JOIN products p ON poi.product_id = p.id
            JOIN purchase_orders po ON poi.po_id = po.id
            LEFT JOIN locations l ON rr.location_id = l.id
            LEFT JOIN users u ON rr.received_by = u.id
            ORDER BY rr.received_date DESC
        `);
        return rows;
    }

    // Mendapatkan catatan penerimaan berdasarkan ID
    static async findById(id) {
        const [rows] = await pool.query(`
            SELECT
                rr.id,
                rr.po_item_id,
                rr.received_quantity,
                rr.received_date,
                rr.batch_number,
                rr.expiry_date,
                rr.location_id,
                rr.qc_status,
                rr.notes,
                poi.product_id,
                poi.po_id,
                p.sku AS product_sku,
                p.name AS product_name,
                po.po_number,
                l.location_code,
                u.full_name AS received_by_user
            FROM receiving_records rr
            JOIN purchase_order_items poi ON rr.po_item_id = poi.id
            JOIN products p ON poi.product_id = p.id
            JOIN purchase_orders po ON poi.po_id = po.id
            LEFT JOIN locations l ON rr.location_id = l.id
            LEFT JOIN users u ON rr.received_by = u.id
            WHERE rr.id = ?
        `, [id]);
        return rows[0];
    }

    // Membuat catatan penerimaan baru
    static async create({ po_item_id, received_quantity, received_by, batch_number, expiry_date, location_id, qc_status, notes }) {
        const [result] = await pool.query(
            'INSERT INTO receiving_records (po_item_id, received_quantity, received_by, batch_number, expiry_date, location_id, qc_status, notes) VALUES (?, ?, ?, ?, ?, ?, ?, ?)',
            [po_item_id, received_quantity, received_by, batch_number || null, expiry_date || null, location_id || null, qc_status, notes || null]
        );
        return result.insertId;
    }

    // Memperbarui catatan penerimaan (misal: update QC status atau quantity jika ada koreksi)
    static async update(id, { received_quantity, batch_number, expiry_date, location_id, qc_status, notes }) {
        const [result] = await pool.query(
            'UPDATE receiving_records SET received_quantity = ?, batch_number = ?, expiry_date = ?, location_id = ?, qc_status = ?, notes = ? WHERE id = ?',
            [received_quantity, batch_number || null, expiry_date || null, location_id || null, qc_status, notes || null, id]
        );
        return result.affectedRows;
    }

    // Menghapus catatan penerimaan
    static async delete(id) {
        const [result] = await pool.query('DELETE FROM receiving_records WHERE id = ?', [id]);
        return result.affectedRows;
    }
}

module.exports = ReceivingRecord;
```

Penjelasan Model ReceivingRecord:

    getAll(): Mengambil semua catatan penerimaan, dengan join ke purchase_order_items, products, purchase_orders, locations, dan users untuk mendapatkan detail lengkap dari setiap penerimaan.

    findById(): Mencari catatan penerimaan berdasarkan ID-nya dengan detail lengkap.

    create(): Menambahkan catatan penerimaan baru. Ini akan dipanggil setelah barang fisik diterima.

    update(): Memperbarui detail catatan penerimaan, misalnya jika ada koreksi kuantitas atau perubahan status QC.

    delete(): Menghapus catatan penerimaan.


## Controller ini akan berisi fungsi-fungsi untuk:

    Menampilkan daftar Purchase Orders.

    Menampilkan form untuk membuat Purchase Order baru.

    Memproses pembuatan Purchase Order.

    Menampilkan detail Purchase Order beserta item-itemnya.

    Menampilkan form untuk mencatat penerimaan barang untuk item PO tertentu.

    Memproses pencatatan penerimaan barang, yang akan melibatkan pembaruan received_quantity di purchase_order_items dan penambahan stok ke inventory_stock.

###` wms-app/src/controllers/receivingController.js`
```
// src/controllers/receivingController.js
const PurchaseOrder = require('../models/PurchaseOrder');
const PurchaseOrderItem = require('../models/PurchaseOrderItem');
const Product = require('../models/Product'); // Untuk dropdown produk
const Location = require('../models/Location'); // Untuk dropdown lokasi
const InventoryStock = require('../models/InventoryStock'); // Untuk update stok inventaris
const ReceivingRecord = require('../models/ReceivingRecord'); // Untuk mencatat setiap penerimaan

// --- Fungsi Bantu (Helper Functions) ---

// Fungsi bantu untuk memperbarui total stok produk di tabel products (reuse dari inventoryStockController)
async function updateProductCurrentStock(productId) {
    try {
        const totalStock = await InventoryStock.getTotalStockByProductId(productId);
        await Product.updateCurrentStock(productId, totalStock);
    } catch (err) {
        console.error(`Error updating current stock for Product ID ${productId}:`, err);
    }
}

// Fungsi bantu untuk memperbarui status PO berdasarkan item PO yang diterima
async function updatePurchaseOrderStatus(poId) {
    try {
        const poItems = await PurchaseOrderItem.getItemsByPoId(poId);
        let allItemsReceived = true;
        let anyItemReceived = false;

        for (const item of poItems) {
            if (item.received_quantity > 0) {
                anyItemReceived = true;
            }
            if (item.received_quantity < item.ordered_quantity) {
                allItemsReceived = false;
            }
        }

        let newStatus;
        if (allItemsReceived) {
            newStatus = 'Received';
        } else if (anyItemReceived) {
            newStatus = 'Partially Received';
        } else {
            newStatus = 'Issued'; // Atau 'Pending' jika Anda memulai dari status itu
        }

        await PurchaseOrder.updateStatus(poId, newStatus);
    } catch (err) {
        console.error(`Error updating PO status for PO ID ${poId}:`, err);
    }
}

// --- Controller Functions ---

// 1. PO Management
// Menampilkan daftar Purchase Orders
exports.listPurchaseOrders = async (req, res) => {
    try {
        const purchaseOrders = await PurchaseOrder.getAll();
        const message = req.query.message || null;
        const error = req.query.error || null;
        res.render('admin/receiving/purchase_orders/list', {
            purchaseOrders,
            user: req.user,
            message,
            error
        });
    } catch (err) {
        console.error('Error fetching purchase orders:', err);
        res.render('error', { message: 'Gagal mengambil data Purchase Orders.' });
    }
};

// Menampilkan form untuk membuat PO baru
exports.showAddPurchaseOrderForm = async (req, res) => {
    try {
        const products = await Product.getAll(); // Untuk dropdown item PO
        res.render('admin/receiving/purchase_orders/add', {
            user: req.user,
            products,
            error: null
        });
    } catch (err) {
        console.error('Error showing add PO form:', err);
        res.render('error', { message: 'Gagal menampilkan form tambah Purchase Order.' });
    }
};

// Memproses pembuatan PO baru
exports.addPurchaseOrder = async (req, res) => {
    const { po_number, order_date, supplier_name, expected_delivery_date, notes, products } = req.body;

    if (!po_number || !order_date || !supplier_name || !products || products.length === 0) {
        const availableProducts = await Product.getAll();
        return res.render('admin/receiving/purchase_orders/add', {
            user: req.user,
            products: availableProducts,
            error: 'Nomor PO, Tanggal Order, Pemasok, dan setidaknya satu Produk harus diisi.'
        });
    }

    try {
        // Cek apakah PO Number sudah ada
        const existingPO = await PurchaseOrder.findByPoNumber(po_number);
        if (existingPO) {
            const availableProducts = await Product.getAll();
            return res.render('admin/receiving/purchase_orders/add', {
                user: req.user,
                products: availableProducts,
                error: 'Nomor PO ini sudah ada. Harap gunakan nomor PO yang lain.'
            });
        }

        // Buat PO baru
        const poId = await PurchaseOrder.create({
            po_number,
            order_date,
            supplier_name,
            expected_delivery_date,
            notes
        });

        // Tambahkan item-item ke PO
        for (const item of products) {
            if (item.product_id && item.ordered_quantity > 0) {
                await PurchaseOrderItem.create({
                    po_id: poId,
                    product_id: parseInt(item.product_id),
                    ordered_quantity: parseFloat(item.ordered_quantity),
                    unit_of_measure: item.unit_of_measure // Asumsikan unit_of_measure dikirim dari form atau diambil dari produk
                });
            }
        }

        res.redirect('/admin/receiving/purchase-orders?message=Purchase Order berhasil dibuat!');
    } catch (err) {
        console.error('Error adding purchase order:', err);
        const availableProducts = await Product.getAll();
        res.render('admin/receiving/purchase_orders/add', {
            user: req.user,
            products: availableProducts,
            error: 'Gagal membuat Purchase Order. Terjadi kesalahan server.'
        });
    }
};

// Menampilkan detail PO dan item-itemnya, serta opsi untuk menerima barang
exports.showPurchaseOrderDetail = async (req, res) => {
    try {
        const poId = req.params.id;
        const purchaseOrder = await PurchaseOrder.findById(poId);
        if (!purchaseOrder) {
            return res.redirect('/admin/receiving/purchase-orders?error=Purchase Order tidak ditemukan.');
        }

        const poItems = await PurchaseOrderItem.getItemsByPoId(poId);
        const locations = await Location.getAll(); // Untuk dropdown lokasi penerimaan

        const message = req.query.message || null;
        const error = req.query.error || null;

        res.render('admin/receiving/purchase_orders/detail', {
            purchaseOrder,
            poItems,
            locations, // Diperlukan untuk form penerimaan di detail
            user: req.user,
            message,
            error
        });
    } catch (err) {
        console.error('Error fetching PO detail:', err);
        res.render('error', { message: 'Gagal mengambil detail Purchase Order.' });
    }
};


// 2. Receiving Process
// Memproses pencatatan penerimaan barang
exports.receiveItem = async (req, res) => {
    const poItemId = req.params.poItemId;
    const { received_quantity, batch_number, expiry_date, location_id, qc_status, notes } = req.body;

    try {
        const poItem = await PurchaseOrderItem.findById(poItemId);
        if (!poItem) {
            return res.redirect(`/admin/receiving/purchase-orders?error=Item PO tidak ditemukan.`);
        }

        const poId = poItem.po_id;
        const productId = poItem.product_id;
        const orderedQuantity = poItem.ordered_quantity;
        const currentReceivedQuantity = poItem.received_quantity;

        if (!received_quantity || parseFloat(received_quantity) <= 0) {
            return res.redirect(`/admin/receiving/purchase-orders/${poId}/detail?error=Kuantitas diterima harus lebih dari 0.`);
        }

        const quantityToReceive = parseFloat(received_quantity);

        // Validasi: tidak boleh menerima lebih dari yang dipesan (ordered - already received)
        if (currentReceivedQuantity + quantityToReceive > orderedQuantity) {
            return res.redirect(`/admin/receiving/purchase-orders/${poId}/detail?error=Kuantitas yang diterima melebihi kuantitas yang dipesan untuk item ini.`);
        }

        // 1. Buat catatan penerimaan (receiving record)
        const receivedById = req.user ? req.user.id : null; // Ambil ID pengguna yang login
        await ReceivingRecord.create({
            po_item_id: poItemId,
            received_quantity: quantityToReceive,
            received_by: receivedById,
            batch_number,
            expiry_date,
            location_id: parseInt(location_id), // Lokasi penerimaan awal
            qc_status: qc_status || 'Pending', // Default 'Pending' jika tidak ada status QC
            notes
        });

        // 2. Perbarui received_quantity di purchase_order_items
        await PurchaseOrderItem.updateReceivedQuantity(poItemId, quantityToReceive);

        // 3. Tambahkan stok ke inventory_stock
        // Cek apakah sudah ada stok produk yang sama di lokasi yang sama dengan batch_number yang sama
        let existingStocks = await InventoryStock.findByProductAndLocation(productId, parseInt(location_id));

        let foundExistingBatch = false;
        if (existingStocks && existingStocks.length > 0) {
            for (let stock of existingStocks) {
                if ((batch_number && stock.batch_number === batch_number) || (!batch_number && !stock.batch_number)) {
                    await InventoryStock.updateQuantity(stock.id, stock.quantity + quantityToReceive);
                    foundExistingBatch = true;
                    break;
                }
            }
        }

        if (!foundExistingBatch) {
            // Jika tidak ditemukan batch yang sama atau belum ada stok di lokasi ini
            await InventoryStock.create({
                product_id: productId,
                location_id: parseInt(location_id),
                quantity: quantityToReceive,
                batch_number: batch_number || null,
                expiry_date: expiry_date || null
            });
        }

        // 4. Perbarui total stok produk di tabel products (total keseluruhan)
        await updateProductCurrentStock(productId);

        // 5. Perbarui status Purchase Order
        await updatePurchaseOrderStatus(poId);

        res.redirect(`/admin/receiving/purchase-orders/${poId}/detail?message=Barang berhasil diterima dan stok diperbarui!`);

    } catch (err) {
        console.error('Error receiving item:', err);
        // Kembali ke halaman detail PO dengan pesan error
        const poItem = await PurchaseOrderItem.findById(poItemId); // Ambil ulang poItem untuk mendapatkan poId
        const poId = poItem ? poItem.po_id : null;
        if (poId) {
            return res.redirect(`/admin/receiving/purchase-orders/${poId}/detail?error=Gagal mencatat penerimaan barang. Terjadi kesalahan server.`);
        }
        res.render('error', { message: 'Gagal mencatat penerimaan barang. Terjadi kesalahan server.' });
    }
};
```

Penjelasan Utama di receivingController.js:

Helper Functions (updateProductCurrentStock, updatePurchaseOrderStatus):

    updateProductCurrentStock: Fungsi pembantu yang kita reuse dari inventoryStockController untuk memastikan current_stock_quantity di tabel products selalu akurat.

    updatePurchaseOrderStatus: Fungsi baru yang sangat penting. Ini akan memeriksa semua item dalam PO. Jika semua item sudah diterima penuh, status PO akan menjadi 'Received'. Jika hanya sebagian item yang diterima, status akan menjadi 'Partially Received'. Jika belum ada yang diterima, status tetap 'Issued' (atau 'Pending').

PO Management Functions (listPurchaseOrders, showAddPurchaseOrderForm, addPurchaseOrder, showPurchaseOrderDetail):

    Ini adalah fungsi-fungsi CRUD dasar untuk mengelola Purchase Orders.

    addPurchaseOrder: Ketika PO baru dibuat, itu juga akan membuat purchase_order_items yang terkait.

    showPurchaseOrderDetail: Menampilkan PO beserta semua itemnya, dan juga akan menjadi tempat di mana form untuk mencatat penerimaan barang akan diletakkan.

Receiving Process (receiveItem): Ini adalah inti dari modul penerimaan:

    Menerima poItemId dan detail penerimaan (kuantitas, batch, dll.) dari form.

    Validasi: Memastikan kuantitas yang diterima valid dan tidak melebihi kuantitas yang dipesan (termasuk yang sudah diterima).

    Langkah 1: Buat Catatan Penerimaan: Membuat entri baru di tabel receiving_records.

    Langkah 2: Perbarui received_quantity di purchase_order_items: Menambahkan kuantitas yang baru diterima ke received_quantity di item PO yang relevan.

    Langkah 3: Tambahkan Stok ke inventory_stock: Menggunakan logika yang sudah ada untuk menambahkan/menggabungkan stok di tabel inventory_stock berdasarkan produk, lokasi, dan batch.

    Langkah 4: Perbarui Total Stok Produk: Memanggil updateProductCurrentStock untuk memastikan current_stock_quantity di tabel products diperbarui.

    Langkah 5: Perbarui Status PO: Memanggil updatePurchaseOrderStatus untuk memperbarui status PO secara otomatis.


### `wms-app/src/routes/receivingRoutes.js`
```// src/routes/receivingRoutes.js
const express = require('express');
const router = express.Router();
const receivingController = require('../controllers/receivingController');
const authMiddleware = require('../middleware/authMiddleware'); // Pastikan path ini benar

// Middleware untuk otorisasi (misalnya, hanya admin atau ppic yang bisa akses)
// Sesuaikan peran yang diizinkan sesuai kebutuhan Anda
const authorize = (roles) => authMiddleware.authorize(roles);

// --- Purchase Order (PO) Routes ---

// Menampilkan daftar Purchase Orders
router.get('/purchase-orders', authorize(['admin', 'ppic', 'gudang']), receivingController.listPurchaseOrders);

// Menampilkan form untuk membuat PO baru
router.get('/purchase-orders/add', authorize(['admin', 'ppic']), receivingController.showAddPurchaseOrderForm);

// Memproses pembuatan PO baru
router.post('/purchase-orders/add', authorize(['admin', 'ppic']), receivingController.addPurchaseOrder);

// Menampilkan detail PO dan opsi untuk menerima barang
router.get('/purchase-orders/:id/detail', authorize(['admin', 'ppic', 'gudang', 'qc']), receivingController.showPurchaseOrderDetail);

// --- Receiving Process Routes ---

// Memproses pencatatan penerimaan barang untuk item PO tertentu
// URL: /admin/receiving/purchase-orders/:poId/items/:poItemId/receive
router.post('/purchase-orders/:poId/items/:poItemId/receive', authorize(['admin', 'gudang', 'qc']), receivingController.receiveItem);

// TODO: Tambahkan rute untuk edit/delete PO atau item PO jika diperlukan nanti

module.exports = router;
```

Penjelasan Routes:

    Import yang Diperlukan: Mengimpor express, router, receivingController, dan authMiddleware.

    authorize Middleware: Menggunakan helper authorize dari authMiddleware untuk membatasi akses berdasarkan peran pengguna. Sesuaikan peran yang diizinkan (admin, ppic, gudang, qc) sesuai dengan kebutuhan WMS Anda.

    Struktur URL: Saya telah membuat struktur URL yang logis di bawah /admin/receiving/ (kita akan menambahkan ini di src/app.js).

    /purchase-orders: Untuk manajemen PO secara umum.

    /purchase-orders/add: Untuk form tambah PO.

    /purchase-orders/:id/detail: Untuk detail PO dan form penerimaan.

    /purchase-orders/:poId/items/:poItemId/receive: Ini adalah endpoint untuk POST data saat barang diterima untuk item PO spesifik.

### `wms-app/src/app.js`

Tambahkan require untuk receivingRoutes di bagian atas, bersama dengan route lainnya.

Tambahkan app.use() untuk receivingRoutes
```
// src/app.js
const express = require('express');
const path = require('path');
const cookieParser = require('cookie-parser');
// Pastikan passport dan authMiddleware diimpor dengan benar
const { authenticateToken, authorizeRoles } = require('./middleware/authMiddleware');
const passport = require('./config/passport'); // Pastikan path ini benar
const session = require('express-session'); // Pastikan express-session diimpor
const flash = require('connect-flash'); // Pastikan connect-flash diimpor

const { connectDB } = require('./config/database');
require('dotenv').config();

const app = express();
const PORT = process.env.PORT || 3000;

// Import routes
const authRoutes = require('./routes/authRoutes');
const adminRoutes = require('./routes/adminRoutes'); // Untuk dashboard admin umum
const userRoutes = require('./routes/userRoutes'); // Untuk manajemen users
const productRoutes = require('./routes/productRoutes'); // Untuk produk
const locationRoutes = require('./routes/locationRoutes'); // Untuk lokasi
const inventoryStockRoutes = require('./routes/inventoryStockRoutes'); // Untuk stok inventaris
const receivingRoutes = require('./routes/receivingRoutes'); // Untuk PO dan Receiving (BARU)

// Menghubungkan ke database saat aplikasi dimulai
connectDB();

// Middleware dasar Express
app.use(express.urlencoded({ extended: true })); // Untuk parsing data dari form HTML
app.use(express.json()); // Untuk parsing body JSON
app.use(cookieParser()); // Untuk mem-parsing dan membaca cookies

// Konfigurasi Session
app.use(session({
    secret: process.env.SESSION_SECRET || 'supersecretkey', // Gunakan variabel lingkungan
    resave: false,
    saveUninitialized: false,
    cookie: { maxAge: 24 * 60 * 60 * 1000 } // 24 jam
}));

// Inisialisasi Passport
app.use(passport.initialize());
app.use(passport.session());

// Connect-flash untuk pesan flash
app.use(flash());

// Middleware untuk menyalurkan pesan flash ke semua view
app.use((req, res, next) => {
    res.locals.message = req.flash('message');
    res.locals.error = req.flash('error');
    next();
});


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


// --- MENGGUNAKAN ROUTES YANG SUDAH DIBUAT ---
app.use('/admin', authenticateToken, adminRoutes); // Admin general routes (e.g., /admin/users)
app.use('/admin/users', authenticateToken, authorizeRoles('admin'), userRoutes); // Manajemen pengguna
app.use('/admin/products', authenticateToken, productRoutes); // Produk
app.use('/admin/locations', authenticateToken, locationRoutes); // Lokasi
app.use('/admin/inventory-stocks', authenticateToken, inventoryStockRoutes); // Stok Inventaris
app.use('/admin/receiving', authenticateToken, receivingRoutes); // PO dan Receiving (BARU)


// Middleware penanganan kesalahan umum
app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).send('Ada yang salah!');
});

// Jalankan server
app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
    console.log('\n--- URL Penting ---');
    console.log(`1. Halaman Login: http://localhost:${PORT}/login`);
    console.log(`2. Dashboard Admin (Akses setelah Login): http://localhost:${PORT}/dashboard/admin`);

    console.log('\n    --- Manajemen Pengguna (Akses setelah Login sebagai Admin) ---');
    console.log(`    - Buat Pengguna Baru: http://localhost:${PORT}/admin/users/create`);
    console.log(`    - Lihat Daftar Pengguna: http://localhost:${PORT}/admin/users`);
    console.log(`    - Ubah Password Admin: http://localhost:${PORT}/admin/change-password`);

    console.log('\n    --- Manajemen Produk (Akses setelah Login) ---');
    console.log(`    - Buat Produk Baru: http://localhost:${PORT}/admin/products/create`);
    console.log(`    - Lihat Daftar Produk: http://localhost:${PORT}/admin/products`);

    console.log('\n    --- Manajemen Lokasi (Akses setelah Login) ---');
    console.log(`    - Buat Lokasi Baru: http://localhost:${PORT}/admin/locations/create`);
    console.log(`    - Lihat Daftar Lokasi: http://localhost:${PORT}/admin/locations`);

    console.log('\n    --- Manajemen Inventaris (Akses setelah Login) ---');
    console.log(`    - Tambah Stok Baru (Barang Masuk): http://localhost:${PORT}/admin/inventory-stocks/add`);
    console.log(`    - Lihat Daftar Stok Inventaris: http://localhost:${PORT}/admin/inventory-stocks`);

    console.log('\n    --- Manajemen Penerimaan Barang (Purchase Order & Receiving) (BARU - Akses setelah Login) ---');
    console.log(`    - Buat Purchase Order Baru: http://localhost:${PORT}/admin/receiving/purchase-orders/add`);
    console.log(`    - Lihat Daftar Purchase Orders: http://localhost:${PORT}/admin/receiving/purchase-orders`);
    console.log('--------------------');
});
```

Perbaikan dan Penjelasan:

    Import Tambahan: Saya menambahkan session, flash, dan passport import di bagian atas app.js karena mereka digunakan dalam konfigurasi Express.

    Konfigurasi Session & Flash: Saya sertakan kembali konfigurasi express-session, passport.initialize(), passport.session(), dan connect-flash yang penting untuk manajemen sesi dan pesan.

    Middleware authenticateToken untuk Semua Rute Admin: Saya menambahkan authenticateToken sebagai middleware untuk semua grup route di bawah /admin. Ini memastikan bahwa setiap akses ke halaman admin (produk, lokasi, inventaris, penerimaan) memerlukan autentikasi terlebih dahulu. Ini adalah praktik keamanan yang baik.
        
        app.use('/admin', authenticateToken, adminRoutes);

        app.use('/admin/users', authenticateToken, authorizeRoles('admin'), userRoutes); (Untuk userRoutes saya menambahkan authorizeRoles('admin') karena manajemen pengguna biasanya hanya untuk admin).

        app.use('/admin/products', authenticateToken, productRoutes);

        app.use('/admin/locations', authenticateToken, locationRoutes);

        app.use('/admin/inventory-stocks', authenticateToken, inventoryStockRoutes);

        app.use('/admin/receiving', authenticateToken, receivingRoutes);

    URL Penting di Konsol: Menambahkan URL untuk modul PO dan Receiving ke dalam output konsol saat server dimulai, agar Anda mudah mengaksesnya.

Setelah Anda melakukan perubahan ini di src/app.js dan menyimpannya:

    Matikan server Node.js Anda (dengan Ctrl+C).

    Jalankan ulang server Anda dengan node src/app.js (atau nodemon).

    Pastikan tidak ada error di konsol.


## `wms-app/views/admin/receiving/purchase_orders/list.ejs`
```
<% layout('layouts/admin-layout') %>

<div class="container-fluid">
    <h1 class="h3 mb-4 text-gray-800">Daftar Purchase Orders</h1>

    <div class="card shadow mb-4">
        <div class="card-header py-3 d-flex justify-content-between align-items-center">
            <h6 class="m-0 font-weight-bold text-primary">Data Purchase Orders</h6>
            <a href="/admin/receiving/purchase-orders/add" class="btn btn-primary btn-sm">
                <i class="fas fa-plus"></i> Buat Purchase Order Baru
            </a>
        </div>
        <div class="card-body">
            <% if (message) { %>
                <div class="alert alert-success" role="alert">
                    <%= message %>
                </div>
            <% } %>
            <% if (error) { %>
                <div class="alert alert-danger" role="alert">
                    <%= error %>
                </div>
            <% } %>

            <div class="table-responsive">
                <table class="table table-bordered" id="dataTable" width="100%" cellspacing="0">
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>No. PO</th>
                            <th>Tanggal Order</th>
                            <th>Pemasok</th>
                            <th>Tgl. Estimasi Kirim</th>
                            <th>Status</th>
                            <th>Aksi</th>
                        </tr>
                    </thead>
                    <tbody>
                        <% if (purchaseOrders && purchaseOrders.length > 0) { %>
                            <% purchaseOrders.forEach(po => { %>
                                <tr>
                                    <td><%= po.id %></td>
                                    <td><%= po.po_number %></td>
                                    <td><%= po.order_date ? new Date(po.order_date).toLocaleDateString('id-ID') : '-' %></td>
                                    <td><%= po.supplier_name %></td>
                                    <td><%= po.expected_delivery_date ? new Date(po.expected_delivery_date).toLocaleDateString('id-ID') : '-' %></td>
                                    <td>
                                        <% if (po.status === 'Received') { %>
                                            <span class="badge badge-success"><%= po.status %></span>
                                        <% } else if (po.status === 'Partially Received') { %>
                                            <span class="badge badge-warning"><%= po.status %></span>
                                        <% } else if (po.status === 'Cancelled') { %>
                                            <span class="badge badge-danger"><%= po.status %></span>
                                        <% } else { %>
                                            <span class="badge badge-info"><%= po.status %></span>
                                        <% } %>
                                    </td>
                                    <td>
                                        <a href="/admin/receiving/purchase-orders/<%= po.id %>/detail" class="btn btn-info btn-sm">
                                            <i class="fas fa-eye"></i> Detail
                                        </a>
                                        <% // Anda bisa menambahkan tombol edit/delete PO di sini nanti %>
                                    </td>
                                </tr>
                            <% }) %>
                        <% } else { %>
                            <tr>
                                <td colspan="7" class="text-center">Tidak ada Purchase Order ditemukan.</td>
                            </tr>
                        <% } %>
                    </tbody>
                </table>
            </div>
        </div>
    </div>
</div>
```

Penjelasan list.ejs:

    Menggunakan layout admin-layout.

    Menampilkan judul, tombol "Buat Purchase Order Baru".

    Menampilkan pesan sukses/error (dari connect-flash).

    Tabel untuk menampilkan data PO: ID, No. PO, Tanggal Order, Pemasok, Tanggal Estimasi Kirim, Status.

    Status PO ditampilkan dengan badge warna yang berbeda untuk memudahkan visualisasi.

    Tombol "Detail" akan mengarahkan ke halaman detail PO.

## `wms-app/views/admin/receiving/purchase_orders/add.ejs
```
<% layout('layouts/admin-layout') %>

<div class="container-fluid">
    <h1 class="h3 mb-4 text-gray-800">Buat Purchase Order Baru</h1>

    <div class="card shadow mb-4">
        <div class="card-header py-3">
            <h6 class="m-0 font-weight-bold text-primary">Form Purchase Order</h6>
        </div>
        <div class="card-body">
            <% if (error) { %>
                <div class="alert alert-danger" role="alert">
                    <%= error %>
                </div>
            <% } %>

            <form action="/admin/receiving/purchase-orders/add" method="POST">
                <div class="row">
                    <div class="col-md-6">
                        <div class="form-group">
                            <label for="po_number">Nomor PO <span class="text-danger">*</span></label>
                            <input type="text" class="form-control" id="po_number" name="po_number" required>
                        </div>
                    </div>
                    <div class="col-md-6">
                        <div class="form-group">
                            <label for="order_date">Tanggal Order <span class="text-danger">*</span></label>
                            <input type="date" class="form-control" id="order_date" name="order_date" required>
                        </div>
                    </div>
                </div>
                <div class="row">
                    <div class="col-md-6">
                        <div class="form-group">
                            <label for="supplier_name">Pemasok <span class="text-danger">*</span></label>
                            <input type="text" class="form-control" id="supplier_name" name="supplier_name" required>
                        </div>
                    </div>
                    <div class="col-md-6">
                        <div class="form-group">
                            <label for="expected_delivery_date">Tanggal Estimasi Pengiriman</label>
                            <input type="date" class="form-control" id="expected_delivery_date" name="expected_delivery_date">
                        </div>
                    </div>
                </div>
                <div class="form-group">
                    <label for="notes">Catatan</label>
                    <textarea class="form-control" id="notes" name="notes" rows="3"></textarea>
                </div>

                <hr>
                <h5 class="mb-3">Item Produk <span class="text-danger">*</span></h5>
                <div id="product-items-container">
                    <div class="product-item row mb-3 border p-3 rounded">
                        <div class="col-md-5">
                            <div class="form-group">
                                <label for="product_id_0">Produk</label>
                                <select class="form-control" id="product_id_0" name="products[0][product_id]" required>
                                    <option value="">Pilih Produk</option>
                                    <% products.forEach(product => { %>
                                        <option value="<%= product.id %>" data-unit="<%= product.unit_of_measure %>">
                                            <%= product.sku %> - <%= product.name %> (<%= product.unit_of_measure %>)
                                        </option>
                                    <% }) %>
                                </select>
                            </div>
                        </div>
                        <div class="col-md-3">
                            <div class="form-group">
                                <label for="ordered_quantity_0">Kuantitas Dipesan</label>
                                <input type="number" step="0.01" class="form-control" id="ordered_quantity_0" name="products[0][ordered_quantity]" min="0.01" required>
                            </div>
                        </div>
                        <div class="col-md-3">
                            <div class="form-group">
                                <label for="unit_of_measure_0">Unit Ukur</label>
                                <input type="text" class="form-control" id="unit_of_measure_0" name="products[0][unit_of_measure]" readonly>
                            </div>
                        </div>
                        <div class="col-md-1 d-flex align-items-end">
                            <button type="button" class="btn btn-danger btn-sm remove-item-btn"><i class="fas fa-minus"></i></button>
                        </div>
                    </div>
                </div>
                <button type="button" class="btn btn-success btn-sm mt-3" id="add-item-btn"><i class="fas fa-plus"></i> Tambah Item</button>

                <div class="mt-4">
                    <button type="submit" class="btn btn-primary">Buat Purchase Order</button>
                    <a href="/admin/receiving/purchase-orders" class="btn btn-secondary">Batal</a>
                </div>
            </form>
        </div>
    </div>
</div>

<script>
    let itemCounter = 1; // Mulai dari 1 karena indeks 0 sudah ada secara default

    document.getElementById('add-item-btn').addEventListener('click', function() {
        const container = document.getElementById('product-items-container');
        const newItem = document.createElement('div');
        newItem.classList.add('product-item', 'row', 'mb-3', 'border', 'p-3', 'rounded');
        newItem.innerHTML = `
            <div class="col-md-5">
                <div class="form-group">
                    <label for="product_id_${itemCounter}">Produk</label>
                    <select class="form-control" id="product_id_${itemCounter}" name="products[${itemCounter}][product_id]" required>
                        <option value="">Pilih Produk</option>
                        <% products.forEach(product => { %>
                            <option value="<%= product.id %>" data-unit="<%= product.unit_of_measure %>">
                                <%= product.sku %> - <%= product.name %> (<%= product.unit_of_measure %>)
                            </option>
                        <% }) %>
                    </select>
                </div>
            </div>
            <div class="col-md-3">
                <div class="form-group">
                    <label for="ordered_quantity_${itemCounter}">Kuantitas Dipesan</label>
                    <input type="number" step="0.01" class="form-control" id="ordered_quantity_${itemCounter}" name="products[${itemCounter}][ordered_quantity]" min="0.01" required>
                </div>
            </div>
            <div class="col-md-3">
                <div class="form-group">
                    <label for="unit_of_measure_${itemCounter}">Unit Ukur</label>
                    <input type="text" class="form-control" id="unit_of_measure_${itemCounter}" name="products[${itemCounter}][unit_of_measure]" readonly>
                </div>
            </div>
            <div class="col-md-1 d-flex align-items-end">
                <button type="button" class="btn btn-danger btn-sm remove-item-btn"><i class="fas fa-minus"></i></button>
            </div>
        `;
        container.appendChild(newItem);

        // Menambahkan event listener untuk select produk yang baru ditambahkan
        newItem.querySelector(`#product_id_${itemCounter}`).addEventListener('change', function() {
            const selectedOption = this.options[this.selectedIndex];
            const unitInput = newItem.querySelector(`#unit_of_measure_${itemCounter}`);
            unitInput.value = selectedOption.dataset.unit || '';
        });

        // Menambahkan event listener untuk tombol hapus item
        newItem.querySelector('.remove-item-btn').addEventListener('click', function() {
            newItem.remove();
        });

        itemCounter++;
    });

    // Event listener untuk item produk yang sudah ada (indeks 0) saat halaman pertama kali dimuat
    document.getElementById('product_id_0').addEventListener('change', function() {
        const selectedOption = this.options[this.selectedIndex];
        document.getElementById('unit_of_measure_0').value = selectedOption.dataset.unit || '';
    });

    // Event listener untuk tombol hapus item yang sudah ada (jika ingin menghapus satu-satunya item)
    document.querySelector('.product-item .remove-item-btn').addEventListener('click', function() {
        if (document.querySelectorAll('.product-item').length > 1) {
            this.closest('.product-item').remove();
        } else {
            alert('Minimal harus ada satu item produk.');
        }
    });

</script>
```

Penjelasan add.ejs:

    Form untuk detail PO (Nomor PO, Tanggal Order, Pemasok, Tanggal Estimasi Kirim, Catatan).

    Bagian "Item Produk" yang dinamis:

    Menggunakan JavaScript untuk memungkinkan penambahan dan penghapusan baris item produk.

    Setiap baris item memiliki dropdown produk (dari data products yang dikirim controller), kuantitas yang dipesan, dan unit of measure yang akan otomatis terisi saat produk dipilih (mengambil dari data-unit di option produk).

    Nama input menggunakan array products[index][field_name] agar Express dapat memparsingnya menjadi array objek.

    Tombol "Buat Purchase Order" dan "Batal".

## `wms-app/views/admin/receiving/purchase_orders/detail.ejs`
```
<% layout('layouts/admin-layout') %>

<div class="container-fluid">
    <h1 class="h3 mb-4 text-gray-800">Detail Purchase Order</h1>

    <div class="card shadow mb-4">
        <div class="card-header py-3 d-flex justify-content-between align-items-center">
            <h6 class="m-0 font-weight-bold text-primary">Detail PO: <%= purchaseOrder.po_number %></h6>
            <a href="/admin/receiving/purchase-orders" class="btn btn-secondary btn-sm">
                <i class="fas fa-arrow-left"></i> Kembali ke Daftar PO
            </a>
        </div>
        <div class="card-body">
            <% if (message) { %>
                <div class="alert alert-success" role="alert">
                    <%= message %>
                </div>
            <% } %>
            <% if (error) { %>
                <div class="alert alert-danger" role="alert">
                    <%= error %>
                </div>
            <% } %>

            <div class="row">
                <div class="col-md-6">
                    <p><strong>Nomor PO:</strong> <%= purchaseOrder.po_number %></p>
                    <p><strong>Tanggal Order:</strong> <%= new Date(purchaseOrder.order_date).toLocaleDateString('id-ID') %></p>
                    <p><strong>Pemasok:</strong> <%= purchaseOrder.supplier_name %></p>
                </div>
                <div class="col-md-6">
                    <p><strong>Tgl. Estimasi Kirim:</strong> <%= purchaseOrder.expected_delivery_date ? new Date(purchaseOrder.expected_delivery_date).toLocaleDateString('id-ID') : '-' %></p>
                    <p><strong>Status PO:</strong>
                        <% if (purchaseOrder.status === 'Received') { %>
                            <span class="badge badge-success"><%= purchaseOrder.status %></span>
                        <% } else if (purchaseOrder.status === 'Partially Received') { %>
                            <span class="badge badge-warning"><%= purchaseOrder.status %></span>
                        <% } else if (purchaseOrder.status === 'Cancelled') { %>
                            <span class="badge badge-danger"><%= purchaseOrder.status %></span>
                        <% } else { %>
                            <span class="badge badge-info"><%= purchaseOrder.status %></span>
                        <% } %>
                    </p>
                    <p><strong>Catatan:</strong> <%= purchaseOrder.notes || '-' %></p>
                </div>
            </div>

            <hr>
            <h5 class="mb-3">Item Produk Dipesan</h5>
            <div class="table-responsive">
                <table class="table table-bordered" width="100%" cellspacing="0">
                    <thead>
                        <tr>
                            <th>Produk (SKU - Nama)</th>
                            <th>Kuantitas Dipesan</th>
                            <th>Kuantitas Diterima</th>
                            <th>Sisa Diterima</th>
                            <th>Aksi Penerimaan</th>
                        </tr>
                    </thead>
                    <tbody>
                        <% if (poItems && poItems.length > 0) { %>
                            <% poItems.forEach(item => { %>
                                <tr>
                                    <td><%= item.sku %> - <%= item.product_name %> (<%= item.unit_of_measure %>)</td>
                                    <td><%= item.ordered_quantity %> <%= item.unit_of_measure %></td>
                                    <td><%= item.received_quantity %> <%= item.unit_of_measure %></td>
                                    <td><%= (item.ordered_quantity - item.received_quantity).toFixed(2) %> <%= item.unit_of_measure %></td>
                                    <td>
                                        <% if (item.received_quantity < item.ordered_quantity) { %>
                                            <button type="button" class="btn btn-success btn-sm receive-item-btn"
                                                    data-po-item-id="<%= item.id %>"
                                                    data-product-name="<%= item.product_name %>"
                                                    data-remaining-quantity="<%= (item.ordered_quantity - item.received_quantity).toFixed(2) %>">
                                                <i class="fas fa-truck-loading"></i> Terima Barang
                                            </button>
                                        <% } else { %>
                                            <span class="badge badge-secondary">Sudah Diterima Penuh</span>
                                        <% } %>
                                    </td>
                                </tr>
                            <% }) %>
                        <% } else { %>
                            <tr>
                                <td colspan="5" class="text-center">Tidak ada item produk dalam Purchase Order ini.</td>
                            </tr>
                        <% } %>
                    </tbody>
                </table>
            </div>
        </div>
    </div>
</div>

<div class="modal fade" id="receiveItemModal" tabindex="-1" role="dialog" aria-labelledby="receiveItemModalLabel" aria-hidden="true">
    <div class="modal-dialog" role="document">
        <div class="modal-content">
            <div class="modal-header">
                <h5 class="modal-title" id="receiveItemModalLabel">Terima Barang untuk <span id="modalProductName"></span></h5>
                <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                    <span aria-hidden="true">&times;</span>
                </button>
            </div>
            <form id="receiveItemForm" method="POST">
                <div class="modal-body">
                    <p>Sisa Kuantitas yang Dipesan: <strong id="modalRemainingQuantity"></strong></p>
                    <div class="form-group">
                        <label for="received_quantity">Kuantitas Diterima <span class="text-danger">*</span></label>
                        <input type="number" step="0.01" class="form-control" id="received_quantity" name="received_quantity" min="0.01" required>
                    </div>
                    <div class="form-group">
                        <label for="location_id">Lokasi Penerimaan <span class="text-danger">*</span></label>
                        <select class="form-control" id="location_id" name="location_id" required>
                            <option value="">Pilih Lokasi</option>
                            <% locations.forEach(location => { %>
                                <option value="<%= location.id %>"><%= location.warehouse_name %> - <%= location.location_code %></option>
                            <% }) %>
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="batch_number">Nomor Batch (Opsional)</label>
                        <input type="text" class="form-control" id="batch_number" name="batch_number">
                    </div>
                    <div class="form-group">
                        <label for="expiry_date">Tanggal Kadaluwarsa (Opsional)</label>
                        <input type="date" class="form-control" id="expiry_date" name="expiry_date">
                    </div>
                    <div class="form-group">
                        <label for="qc_status">Status QC</label>
                        <select class="form-control" id="qc_status" name="qc_status">
                            <option value="Pending">Pending</option>
                            <option value="Passed">Passed</option>
                            <option value="Failed">Failed</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="notes">Catatan Penerimaan (Opsional)</label>
                        <textarea class="form-control" id="notes_receiving" name="notes" rows="3"></textarea>
                    </div>
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-secondary" data-dismiss="modal">Batal</button>
                    <button type="submit" class="btn btn-primary">Simpan Penerimaan</button>
                </div>
            </form>
        </div>
    </div>
</div>

<script>
    document.addEventListener('DOMContentLoaded', function() {
        const receiveItemModal = document.getElementById('receiveItemModal');
        const modalProductName = document.getElementById('modalProductName');
        const modalRemainingQuantity = document.getElementById('modalRemainingQuantity');
        const receivedQuantityInput = document.getElementById('received_quantity');
        const receiveItemForm = document.getElementById('receiveItemForm');
        const poId = '<%= purchaseOrder.id %>'; // Ambil PO ID dari EJS

        document.querySelectorAll('.receive-item-btn').forEach(button => {
            button.addEventListener('click', function() {
                const poItemId = this.dataset.poItemId;
                const productName = this.dataset.productName;
                const remainingQuantity = this.dataset.remainingQuantity;

                modalProductName.textContent = productName;
                modalRemainingQuantity.textContent = `${remainingQuantity} ${poItem.unit_of_measure}`; // Asumsikan unit_of_measure tersedia atau ambil dari data item
                receivedQuantityInput.value = remainingQuantity; // Default value ke sisa kuantitas
                receivedQuantityInput.max = remainingQuantity; // Set max untuk validasi browser

                // Set action form modal dynamically
                receiveItemForm.action = `/admin/receiving/purchase-orders/${poId}/items/${poItemId}/receive`;

                // Reset form fields
                receivedQuantityInput.value = remainingQuantity;
                document.getElementById('batch_number').value = '';
                document.getElementById('expiry_date').value = '';
                document.getElementById('location_id').value = ''; // Reset dropdown
                document.getElementById('qc_status').value = 'Pending';
                document.getElementById('notes_receiving').value = '';

                $(receiveItemModal).modal('show'); // Tampilkan modal (membutuhkan jQuery/Bootstrap JS)
            });
        });
    });
</script>
```

Penjelasan detail.ejs:

    Menampilkan ringkasan detail PO (Nomor PO, Tanggal Order, Pemasok, Status, dll.).

    Tabel Item Produk Dipesan menunjukkan:

        Produk (SKU - Nama)

        Kuantitas Dipesan

        Kuantitas Diterima (dari received_quantity di purchase_order_items)

        Sisa Diterima (perhitungan)

        Aksi Penerimaan: Tombol "Terima Barang" yang akan muncul jika received_quantity < ordered_quantity.
    
    Modal "Terima Barang":

        Ketika tombol "Terima Barang" diklik, sebuah modal (pop-up) akan muncul.

        Modal ini berisi form untuk mencatat detail penerimaan (kuantitas diterima, lokasi, nomor batch, tanggal kedaluwarsa, status QC, catatan).

        Data seperti nama produk dan sisa kuantitas otomatis terisi dari data attributes tombol.

        action dari form dalam modal diatur secara dinamis menggunakan JavaScript agar mengarah ke endpoint receiveItem di controller dengan poItemId yang benar.

    JavaScript: Menggunakan JavaScript untuk mengelola modal, mengisi data secara dinamis, dan mengatur action form. (Pastikan Anda sudah menyertakan Bootstrap JS dan jQuery di admin-layout.ejs jika Anda belum).

## `wms-app/src/config/passport.js`
```
// src/config/passport.js
const passport = require('passport');
const LocalStrategy = require('passport-local').Strategy;
const jwt = require('jsonwebtoken'); // Mungkin belum digunakan, tapi akan berguna
const bcrypt = require('bcryptjs');
const { pool } = require('./database'); // Import koneksi database

passport.use(new LocalStrategy(
    { usernameField: 'username' }, // Nama field untuk username di form login
    async (username, password, done) => {
        try {
            const [rows] = await pool.query('SELECT * FROM users WHERE username = ?', [username]);
            const user = rows[0];

            if (!user) {
                return done(null, false, { message: 'Username tidak ditemukan.' });
            }

            // Bandingkan password
            const isMatch = await bcrypt.compare(password, user.password); // Asumsi kolom password di DB namanya 'password'
            if (!isMatch) {
                return done(null, false, { message: 'Password salah.' });
            }

            // Login berhasil
            return done(null, user);
        } catch (err) {
            return done(err);
        }
    }
));

// Serialize user: menyimpan ID user ke dalam sesi
passport.serializeUser((user, done) => {
    done(null, user.id);
});

// Deserialize user: mengambil user dari database berdasarkan ID di sesi
passport.deserializeUser(async (id, done) => {
    try {
        const [rows] = await pool.query('SELECT * FROM users WHERE id = ?', [id]);
        const user = rows[0];
        done(null, user);
    } catch (err) {
        done(err);
    }
});

module.exports = passport;
```

perlu menginstal passport, passport-local, dan bcryptjs jika belum:
```
npm install passport passport-local bcryptjs
```

Anda perlu menginstal modul express-session
```
npm install express-session
```

Anda perlu menginstal modul connect-flash
```
npm install connect-flash
```

### buat file `wms-app/src/routes/userRoutes.js
```
// src/routes/userRoutes.js
const express = require('express');
const router = express.Router();
const userController = require('../controllers/userController'); // Kita akan membuat ini selanjutnya
const { authenticateToken, authorizeRoles } = require('../middleware/authMiddleware');

// Middleware untuk memastikan hanya admin yang bisa mengakses rute ini
// Karena ini adalah manajemen pengguna, diasumsikan hanya admin yang bisa melakukannya.
// Jika ada peran lain yang boleh mengelola pengguna, tambahkan ke authorizeRoles.
router.use(authenticateToken, authorizeRoles('admin'));

// Rute untuk menampilkan daftar pengguna
router.get('/', userController.listUsers);

// Rute untuk menampilkan form tambah pengguna baru
router.get('/create', userController.showCreateUserForm);

// Rute untuk memproses penambahan pengguna baru
router.post('/create', userController.createUser);

// Rute untuk menampilkan form edit pengguna
router.get('/edit/:id', userController.showEditUserForm);

// Rute untuk memproses pembaruan pengguna
router.post('/edit/:id', userController.updateUser);

// Rute untuk menghapus pengguna
// Menggunakan POST untuk operasi delete yang disarankan untuk mencegah CSRF
router.post('/delete/:id', userController.deleteUser);

module.exports = router;
```

### perbarui
###`wms-app/src/controllers/userController.js`
```
const User = require('../models/User');
const bcrypt = require('bcryptjs');

exports.listUsers = async (req, res) => {
    try {
        const users = await User.getAll();
        // Ambil 'message' dan 'error' dari connect-flash
        const message = req.flash('message'); // Mengambil pesan sukses
        const error = req.flash('error');   // Mengambil pesan error

        res.render('admin/users/list', {
            users,
            user: req.user, // Menggunakan req.user untuk data user yang sedang login (seperti di layout)
            message: message.length ? message : null, // Flash returns an array, check if it has content
            error: error.length ? error : null        // Flash returns an array, check if it has content
        });
    } catch (err) {
        console.error('Error fetching users:', err);
        req.flash('error', 'Gagal mengambil data pengguna.'); // Set flash error
        res.redirect('/dashboard/admin'); // Redirect ke dashboard admin jika error
    }
};

exports.showCreateUserForm = (req, res) => {
    res.render('admin/users/create', {
        user: req.user, // Pastikan user yang login tersedia di view
        error: req.flash('error').length ? req.flash('error') : null, // Ambil error dari flash
        formData: null // formData digunakan untuk mengisi ulang form jika ada error
    });
};

exports.createUser = async (req, res) => {
    const { username, password, full_name, email, role } = req.body;

    // Tambahkan validasi dasar jika field kosong
    if (!username || !password || !full_name || !role) {
        req.flash('error', 'Semua field wajib diisi (Username, Password, Nama Lengkap, Peran).');
        return res.redirect('/admin/users/create');
    }

    try {
        // Validasi username atau email sudah ada
        const existingUser = await User.findByUsername(username);
        if (existingUser) {
            req.flash('error', 'Username sudah digunakan.');
            return res.render('admin/users/create', {
                user: req.user,
                error: req.flash('error').length ? req.flash('error') : null, // Pastikan flash error terambil
                formData: req.body // Kirim kembali data form yang diisi sebelumnya
            });
        }
        
        // Asumsi findByEmail juga ada di User model
        if (email) { // Hanya cek jika email disediakan
            const existingEmailUser = await User.findByEmail(email);
            if (existingEmailUser) {
                req.flash('error', 'Email sudah digunakan.');
                return res.render('admin/users/create', {
                    user: req.user,
                    error: req.flash('error').length ? req.flash('error') : null,
                    formData: req.body
                });
            }
        }
        

        // Hash password
        const saltRounds = 10;
        const password_hash = await bcrypt.hash(password, saltRounds);

        // Buat user baru
        const newUser = {
            username,
            password_hash,
            full_name,
            email: email || null, // Pastikan email bisa null jika tidak diisi
            role,
            is_active: true // Default true
        };

        await User.create(newUser);

        req.flash('message', 'Pengguna berhasil dibuat!'); // Set flash message
        res.redirect('/admin/users'); // Redirect bersih
    } catch (err) {
        console.error('Create user error:', err);
        req.flash('error', 'Terjadi kesalahan saat membuat pengguna.'); // Set flash error
        res.render('admin/users/create', {
            user: req.user,
            error: req.flash('error').length ? req.flash('error') : null, // Pastikan flash error terambil
            formData: req.body
        });
    }
};

exports.showEditUserForm = async (req, res) => {
    const { id } = req.params;

    try {
        const userToEdit = await User.findById(id);
        if (!userToEdit) {
            req.flash('error', 'Pengguna tidak ditemukan.');
            return res.redirect('/admin/users'); // Redirect jika tidak ditemukan
        }
        res.render('admin/users/edit', {
            user: req.user, // User yang sedang login
            userToEdit: userToEdit, // User yang akan diedit
            error: req.flash('error').length ? req.flash('error') : null // Ambil error dari flash
        });

    } catch (err) {
        console.error('Error fetching user for edit:', err); // Log lebih spesifik
        req.flash('error', 'Gagal mengambil data pengguna untuk diedit.'); // Set flash error
        res.redirect('/admin/users'); // Redirect ke daftar pengguna jika error
    }
};

exports.updateUser = async (req, res) => {
    const { id } = req.params;
    const { username, full_name, email, role, is_active } = req.body;

    // Validasi dasar
    if (!full_name || !role) {
        req.flash('error', 'Nama Lengkap dan Peran wajib diisi.');
        return res.redirect(`/admin/users/edit/${id}`);
    }

    try {
        const updateData = {
            // Username seharusnya tidak diupdate kecuali ada alasan kuat (misalnya, fungsi khusus)
            // username, // Komentar ini karena username biasanya tidak diubah setelah dibuat
            full_name,
            email: email || null,
            role,
            is_active: is_active === 'on' // checkbox
        };

        await User.update(id, updateData);

        req.flash('message', 'Pengguna berhasil diperbarui!'); // Set flash message
        res.redirect('/admin/users'); // Redirect bersih
    } catch (err) {
        console.error('Update user error:', err);
        req.flash('error', 'Terjadi kesalahan saat memperbarui pengguna.'); // Set flash error
        // Redirect ke halaman edit dengan error
        res.redirect(`/admin/users/edit/${id}`); // Gunakan redirect, jangan render langsung
    }
};

exports.deleteUser = async (req, res) => {
    const { id } = req.params;

    try {
        // Jangan izinkan menghapus diri sendiri
        if (parseInt(id) === req.user.id) {
            req.flash('error', 'Anda tidak dapat menghapus akun Anda sendiri.');
            return res.redirect('/admin/users');
        }

        await User.delete(id);
        req.flash('message', 'Pengguna berhasil dihapus!'); // Set flash message
        res.redirect('/admin/users'); // Redirect bersih

    } catch (err) {
        console.error('Delete user error:', err);
        req.flash('error', 'Gagal menghapus pengguna. Pastikan tidak ada data terkait.');
        res.redirect('/admin/users');
    }
};

exports.showChangePasswordForm = async (req, res) => {
    res.render('admin/users/change-password', {
        user: req.user,
        error: req.flash('error').length ? req.flash('error') : null,
        success: req.flash('message').length ? req.flash('message') : null
    });
};

exports.changePassword = async (req, res) => {
    const { current_password, new_password, confirm_password } = req.body;

    if (new_password !== confirm_password) {
        req.flash('error', 'Password baru dan konfirmasi password tidak cocok.');
        return res.redirect('/admin/users/change-password');
    }

    try {
        const user = await User.findById(req.user.id); // Dapatkan user yang sedang login
        if (!user) { // Seharusnya tidak terjadi jika sudah authenticateToken
            req.flash('error', 'Pengguna tidak ditemukan.');
            return res.redirect('/login'); // Mungkin redirect ke login
        }

        // Verifikasi password saat ini
        const isMatch = await bcrypt.compare(current_password, user.password_hash);
        if (!isMatch) {
            req.flash('error', 'Password saat ini salah.');
            return res.redirect('/admin/users/change-password');
        }

        // Hash password baru
        const saltRounds = 10;
        const password_hash = await bcrypt.hash(new_password, saltRounds);

        // Update password
        await User.updatePassword(user.id, password_hash);

        req.flash('message', 'Password berhasil diubah.');
        res.redirect('/admin/users/change-password'); // Redirect bersih
    } catch (err) {
        console.error('Change password error:', err);
        req.flash('error', 'Terjadi kesalahan saat mengubah password.');
        res.redirect('/admin/users/change-password');
    }
};
```

Catatan Tambahan untuk userController.js:

    User.findByEmail(email): Saya menambahkan cek findByEmail di createUser jika Anda memiliki metode ini di User model Anda. Jika belum, Anda mungkin ingin menambahkannya.

    user: req.user: Saya memastikan req.user selalu dilewatkan ke view agar Anda dapat menampilkan informasi pengguna yang masuk di layout atau sidebar.

    formData: Di showCreateUserForm dan createUser, saya menyertakan formData yang memungkinkan Anda mengisi kembali input form dengan data yang sebelumnya dimasukkan pengguna jika terjadi error validasi. Ini meningkatkan user experience. Anda perlu menggunakan ini di view EJS (create.ejs).

    Redirect vs Render: Di beberapa tempat, saya mengubah res.render menjadi res.redirect setelah operasi berhasil atau gagal. Ini adalah praktik yang lebih baik (PRG - Post/Redirect/Get) untuk mencegah masalah saat refresh halaman dan memastikan pesan flash berfungsi dengan baik.


### revisi `wms-app/src/controllers/productController.js`
```
// src/controllers/productController.js
const Product = require('../models/Product'); // Import model Product
// const { pool } = require('../config/database'); // Tidak perlu diimpor di sini jika tidak digunakan secara langsung
const InventoryStock = require('../models/InventoryStock'); // Akan digunakan untuk validasi delete

// Fungsi untuk menampilkan daftar produk
exports.listProducts = async (req, res) => {
    try {
        const products = await Product.getAll(); // Ambil semua produk dari database
        const message = req.flash('message'); // Ambil pesan sukses dari connect-flash
        const error = req.flash('error');     // Ambil pesan error dari connect-flash

        res.render('admin/products/list', { // Render view list.ejs di folder admin/products
            products,
            user: req.user, // Pastikan user login diteruskan ke view
            message: message.length ? message : null, // connect-flash mengembalikan array
            error: error.length ? error : null        // connect-flash mengembalikan array
        });
    } catch (err) {
        console.error('Error fetching products:', err);
        req.flash('error', 'Gagal mengambil data produk.'); // Set error via flash
        res.redirect('/admin/dashboard'); // Redirect ke dashboard admin jika error fatal
    }
};

// Fungsi untuk menampilkan form tambah produk baru
exports.showCreateProductForm = (req, res) => {
    res.render('admin/products/create', {
        user: req.user,
        error: req.flash('error').length ? req.flash('error') : null, // Ambil error dari flash
        formData: null // Untuk mengisi ulang form jika ada error validasi
    });
};

// Fungsi untuk memproses penambahan produk baru
exports.createProduct = async (req, res) => {
    const { sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default } = req.body;

    // Validasi input dasar
    if (!sku || !name || !unit_of_measure) {
        req.flash('error', 'SKU, Nama Produk, dan Satuan Unit harus diisi.');
        return res.render('admin/products/create', {
            user: req.user,
            error: req.flash('error').length ? req.flash('error') : null,
            formData: req.body // Kirim kembali data yang sudah diisi
        });
    }

    try {
        // Cek apakah SKU sudah ada
        const existingProduct = await Product.findBySku(sku);
        if (existingProduct) {
            req.flash('error', `SKU "${sku}" sudah ada. Gunakan SKU lain.`);
            return res.render('admin/products/create', {
                user: req.user,
                error: req.flash('error').length ? req.flash('error') : null,
                formData: req.body
            });
        }

        // Buat produk baru
        await Product.create({
            sku,
            name,
            description: description || null,
            category: category || null,
            unit_of_measure,
            min_stock_level: min_stock_level ? parseFloat(min_stock_level) : 0, // Pastikan number
            max_stock_level: max_stock_level ? parseFloat(max_stock_level) : null, // Pastikan number atau null
            location_default: location_default || null
        });

        req.flash('message', 'Produk berhasil ditambahkan!'); // Set message via flash
        res.redirect('/admin/products'); // Redirect bersih
    } catch (err) {
        console.error('Error creating product:', err);
        req.flash('error', 'Gagal menambahkan produk. Terjadi kesalahan server.'); // Set error via flash
        res.render('admin/products/create', {
            user: req.user,
            error: req.flash('error').length ? req.flash('error') : null,
            formData: req.body
        });
    }
};

// Fungsi untuk menampilkan form edit produk
exports.showEditProductForm = async (req, res) => {
    try {
        const product = await Product.findById(req.params.id);
        if (!product) {
            req.flash('error', 'Produk tidak ditemukan.'); // Set error via flash
            return res.redirect('/admin/products'); // Redirect jika tidak ditemukan
        }
        res.render('admin/products/edit', {
            product,
            user: req.user,
            error: req.flash('error').length ? req.flash('error') : null // Ambil error dari flash
        });
    } catch (err) {
        console.error('Error fetching product for edit:', err);
        req.flash('error', 'Gagal mengambil data produk untuk diedit.'); // Set error via flash
        res.redirect('/admin/products'); // Redirect jika error
    }
};

// Fungsi untuk memproses pembaruan produk
exports.updateProduct = async (req, res) => {
    const { id } = req.params;
    const { sku, name, description, category, unit_of_measure, min_stock_level, max_stock_level, location_default, is_active } = req.body;
    // Konversi string 'true'/'false' dari checkbox menjadi boolean (atau null jika tidak ada)
    const isActiveBoolean = is_active === 'on' || is_active === 'true'; // 'on' for form checkbox, 'true' for direct boolean

    // Validasi input dasar
    if (!sku || !name || !unit_of_measure) {
        req.flash('error', 'SKU, Nama Produk, dan Satuan Unit harus diisi.');
        return res.redirect(`/admin/products/edit/${id}`); // Redirect dengan flash error
    }

    try {
        // Cek apakah SKU baru sudah digunakan oleh produk lain (kecuali produk ini sendiri)
        const existingProduct = await Product.findBySku(sku);
        if (existingProduct && existingProduct.id !== parseInt(id)) {
            req.flash('error', `SKU "${sku}" sudah digunakan oleh produk lain.`);
            return res.redirect(`/admin/products/edit/${id}`); // Redirect dengan flash error
        }

        await Product.update(id, {
            sku,
            name,
            description: description || null,
            category: category || null,
            unit_of_measure,
            min_stock_level: min_stock_level ? parseFloat(min_stock_level) : 0,
            max_stock_level: max_stock_level ? parseFloat(max_stock_level) : null,
            location_default: location_default || null,
            is_active: isActiveBoolean
        });

        req.flash('message', 'Produk berhasil diperbarui!'); // Set message via flash
        res.redirect('/admin/products'); // Redirect bersih
    } catch (err) {
        console.error('Error updating product:', err);
        req.flash('error', 'Gagal memperbarui produk. Terjadi kesalahan server.'); // Set error via flash
        res.redirect(`/admin/products/edit/${id}`); // Redirect dengan flash error
    }
};

// Fungsi untuk memproses penghapusan produk
exports.deleteProduct = async (req, res) => {
    const { id } = req.params;
    try {
        const product = await Product.findById(id);
        if (!product) {
            req.flash('error', 'Produk tidak ditemukan.');
            return res.redirect('/admin/products');
        }

        // Contoh validasi: Cek apakah produk memiliki stok terkait
        const stockCount = await InventoryStock.countStockByProductId(id);
        if (stockCount > 0) {
            req.flash('error', 'Tidak dapat menghapus produk ini karena masih memiliki stok inventaris.');
            return res.redirect('/admin/products');
        }

        // Tambahkan validasi untuk Purchase Order Items jika diperlukan
        // const poItemCount = await PurchaseOrderItem.countItemsByProductId(id);
        // if (poItemCount > 0) {
        //     req.flash('error', 'Tidak dapat menghapus produk ini karena ada dalam Purchase Order.');
        //     return res.redirect('/admin/products');
        // }


        await Product.delete(id);
        req.flash('message', 'Produk berhasil dihapus!'); // Set message via flash
        res.redirect('/admin/products'); // Redirect bersih
    } catch (err) {
        console.error('Error deleting product:', err);
        req.flash('error', 'Gagal menghapus produk. Terjadi kesalahan server atau ada data terkait.'); // Set error via flash
        res.redirect('/admin/products'); // Redirect dengan flash error
    }
};
```

Perubahan Utama:

    Penggunaan req.flash('message') dan req.flash('error'): Semua tempat di mana Anda sebelumnya membaca atau menulis req.query untuk pesan status, sekarang menggunakan req.flash().

    Redirect Bersih: Setelah operasi POST (create, update, delete) atau ketika ada pengalihan karena data tidak ditemukan/validasi gagal, saya mengganti res.render langsung dengan req.flash() diikuti res.redirect(). Ini adalah pola Post/Redirect/Get (PRG) yang lebih baik untuk menghindari isu form resubmission saat refresh dan memastikan pesan flash tampil dengan benar.

    Pengecekan message.length / error.length: req.flash() mengembalikan array. Jika tidak ada pesan, itu akan mengembalikan array kosong []. Saya menambahkan message.length ? message : null agar views Anda tidak perlu berurusan dengan array kosong dan bisa langsung mengecek apakah message atau error memiliki nilai.

    formData di createProduct: Saya menambahkan formData: req.body saat render ulang form create setelah error validasi, agar input yang sudah diisi pengguna tidak hilang. Anda perlu menyesuaikan create.ejs Anda untuk menggunakan formData ini (misalnya <input value="<%= formData ? formData.sku : '' %>">).

    Validasi min_stock_level dan max_stock_level: Saya menambahkan konversi ke parseFloat dan default 0/null untuk memastikan tipe data yang benar.

    Validasi is_active: Menyesuaikan logika untuk checkbox agar menghasilkan boolean yang benar ('on' jika dicentang dari form HTML).

    Penambahan InventoryStock import: Untuk contoh validasi saat menghapus produk, di mana kita mungkin tidak ingin menghapus produk jika masih ada stok terkait. Ini akan memerlukan metode countStockByProductId di model InventoryStock Anda.


### `wms-app/src/routes/locationRoutes.js
```
// src/routes/locationRoutes.js
const express = require('express');
const router = express.Router();
const locationController = require('../controllers/locationController'); // We will create this
const { authenticateToken, authorizeRoles } = require('../middleware/authMiddleware');

// Middleware to protect all location routes, only specific roles can access
// Adjust allowed roles (e.g., admin, gudang, ppic)
router.use(authenticateToken, authorizeRoles('admin', 'gudang', 'ppic'));

// Route to display the list of locations
router.get('/', locationController.listLocations);

// Route to display the form for adding a new location
router.get('/create', authorizeRoles('admin'), locationController.showCreateLocationForm);

// Route to process adding a new location
router.post('/create', authorizeRoles('admin'), locationController.createLocation);

// Route to display the form for editing a location
router.get('/edit/:id', authorizeRoles('admin'), locationController.showEditLocationForm);

// Route to process updating a location
router.post('/edit/:id', authorizeRoles('admin'), locationController.updateLocation);

// Route to delete a location
router.post('/delete/:id', authorizeRoles('admin'), locationController.deleteLocation);

module.exports = router;
```

### revisi `wms-app/src/controllers/locationController.js
```
// src/controllers/locationController.js
const Location = require('../models/Location'); // Import model Location
const { pool } = require('../config/database'); // Untuk query langsung di validasi delete

// Fungsi untuk menampilkan daftar lokasi
exports.listLocations = async (req, res) => {
    try {
        const locations = await Location.getAll(); // Ambil semua lokasi dari database
        const message = req.flash('message'); // Ambil pesan sukses dari connect-flash
        const error = req.flash('error');     // Ambil pesan error dari connect-flash

        res.render('admin/locations/list', { // Render view list.ejs di folder admin/locations
            locations,
            user: req.user,
            message: message.length ? message : null, // connect-flash mengembalikan array
            error: error.length ? error : null        // connect-flash mengembalikan array
        });
    } catch (err) {
        console.error('Error fetching locations:', err);
        req.flash('error', 'Gagal mengambil data lokasi.'); // Set error via flash
        res.redirect('/admin/dashboard'); // Redirect ke dashboard admin jika error fatal
    }
};

// Fungsi untuk menampilkan form tambah lokasi baru
exports.showCreateLocationForm = async (req, res) => {
    try {
        const parentLocations = await Location.getAll(); // Ambil semua lokasi untuk opsi parent
        const locationTypes = ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'];

        res.render('admin/locations/create', {
            user: req.user,
            parentLocations: parentLocations,
            locationTypes: locationTypes,
            error: req.flash('error').length ? req.flash('error') : null, // Ambil error dari flash
            formData: null // Untuk mengisi ulang form jika ada error validasi
        });
    } catch (err) {
        console.error('Error showing create location form:', err);
        req.flash('error', 'Gagal menampilkan form penambahan lokasi.'); // Set error via flash
        res.redirect('/admin/dashboard'); // Redirect jika ada masalah umum
    }
};

// Fungsi untuk memproses penambahan lokasi baru
exports.createLocation = async (req, res) => {
    const { warehouse_name, location_code, zone_area, aisle, rack, shelf_level, bin_box_pallet,
        location_type, parent_location_id, capacity, description } = req.body;

    // Validasi input dasar
    if (!warehouse_name || !location_code || !location_type) {
        req.flash('error', 'Nama Gudang, Kode Lokasi, dan Tipe Lokasi harus diisi.');
        // Ambil lagi data parentLocations dan locationTypes untuk di-render ulang jika ada error
        // Jangan panggil await lagi, karena akan dilakukan di catch block atau redirect
        return res.render('admin/locations/create', {
            user: req.user,
            parentLocations: await Location.getAll(), // Re-fetch for consistent data
            locationTypes: ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'],
            error: req.flash('error').length ? req.flash('error') : null, // Ambil error dari flash
            formData: req.body // Kirim kembali data form yang diisi sebelumnya
        });
    }

    try {
        // Cek apakah kode lokasi sudah ada
        const existingLocation = await Location.findByLocationCode(location_code);
        if (existingLocation) {
            req.flash('error', `Kode Lokasi "${location_code}" sudah ada. Gunakan kode lain.`);
            return res.render('admin/locations/create', {
                user: req.user,
                parentLocations: await Location.getAll(),
                locationTypes: ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'],
                error: req.flash('error').length ? req.flash('error') : null,
                formData: req.body
            });
        }

        await Location.create({
            warehouse_name,
            location_code,
            zone_area: zone_area || null,
            aisle: aisle || null,
            rack: rack || null,
            shelf_level: shelf_level || null,
            bin_box_pallet: bin_box_pallet || null,
            location_type,
            parent_location_id: parent_location_id === '' ? null : parseInt(parent_location_id), // Konversi ke int, set null jika kosong
            capacity: capacity === '' ? null : parseInt(capacity), // Konversi ke int, set null jika kosong
            description: description || null
        });

        req.flash('message', 'Lokasi berhasil ditambahkan!'); // Set message via flash
        res.redirect('/admin/locations'); // Redirect bersih
    } catch (err) {
        console.error('Error creating location:', err);
        req.flash('error', 'Gagal menambahkan lokasi. Terjadi kesalahan server.'); // Set error via flash
        // Re-render form dengan data dan error jika ada masalah database
        res.render('admin/locations/create', {
            user: req.user,
            parentLocations: await Location.getAll(),
            locationTypes: ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'],
            error: req.flash('error').length ? req.flash('error') : null,
            formData: req.body
        });
    }
};

// Fungsi untuk menampilkan form edit lokasi
exports.showEditLocationForm = async (req, res) => {
    try {
        const location = await Location.findById(req.params.id);
        if (!location) {
            req.flash('error', 'Lokasi tidak ditemukan.'); // Set error via flash
            return res.redirect('/admin/locations'); // Redirect jika tidak ditemukan
        }
        const parentLocations = await Location.getAll(); // Ambil semua lokasi untuk opsi parent
        const locationTypes = ['Warehouse', 'Zone', 'Aisle', 'Rack', 'Shelf', 'Bin', 'Pallet'];

        res.render('admin/locations/edit', {
            location,
            user: req.user,
            parentLocations: parentLocations,
            locationTypes: locationTypes,
            error: req.flash('error').length ? req.flash('error') : null // Ambil error dari flash
        });
    } catch (err) {
        console.error('Error fetching location for edit:', err);
        req.flash('error', 'Gagal mengambil data lokasi untuk diedit.'); // Set error via flash
        res.redirect('/admin/locations'); // Redirect jika error
    }
};

// Fungsi untuk memproses pembaruan lokasi
exports.updateLocation = async (req, res) => {
    const { id } = req.params;
    const { warehouse_name, location_code, zone_area, aisle, rack, shelf_level, bin_box_pallet,
        location_type, parent_location_id, capacity, description, is_active } = req.body;
    const isActiveBoolean = is_active === 'on'; // Konversi string 'on' dari checkbox menjadi boolean

    // Validasi input dasar
    if (!warehouse_name || !location_code || !location_type) {
        req.flash('error', 'Nama Gudang, Kode Lokasi, dan Tipe Lokasi harus diisi.');
        return res.redirect(`/admin/locations/edit/${id}`); // Redirect dengan flash error
    }

    try {
        // Cek apakah kode lokasi baru sudah digunakan oleh lokasi lain (kecuali lokasi ini sendiri)
        const existingLocation = await Location.findByLocationCode(location_code);
        if (existingLocation && existingLocation.id !== parseInt(id)) {
            req.flash('error', `Kode Lokasi "${location_code}" sudah digunakan oleh lokasi lain.`);
            return res.redirect(`/admin/locations/edit/${id}`); // Redirect dengan flash error
        }

        await Location.update(id, {
            warehouse_name,
            location_code,
            zone_area: zone_area || null,
            aisle: aisle || null,
            rack: rack || null,
            shelf_level: shelf_level || null,
            bin_box_pallet: bin_box_pallet || null,
            location_type,
            parent_location_id: parent_location_id === '' ? null : parseInt(parent_location_id),
            capacity: capacity === '' ? null : parseInt(capacity),
            description: description || null,
            is_active: isActiveBoolean
        });

        req.flash('message', 'Lokasi berhasil diperbarui!'); // Set message via flash
        res.redirect('/admin/locations'); // Redirect bersih
    } catch (err) {
        console.error('Error updating location:', err);
        req.flash('error', 'Gagal memperbarui lokasi. Terjadi kesalahan server.'); // Set error via flash
        res.redirect(`/admin/locations/edit/${id}`); // Redirect dengan flash error
    }
};

// Fungsi untuk memproses penghapusan lokasi
exports.deleteLocation = async (req, res) => {
    const { id } = req.params;
    try {
        const location = await Location.findById(id);
        if (!location) {
            req.flash('error', 'Lokasi tidak ditemukan.');
            return res.redirect('/admin/locations');
        }

        // Validasi: Cek apakah ada child locations
        const children = await Location.getChildren(id); // Memerlukan method getChildren di model Location
        if (children && children.length > 0) {
            req.flash('error', 'Tidak dapat menghapus lokasi ini karena memiliki lokasi anak.');
            return res.redirect('/admin/locations');
        }

        // Validasi: Cek apakah ada inventory_stock di lokasi ini
        // Menggunakan pool.query langsung karena InventoryStock model belum tentu punya method ini
        const [stockCountRows] = await pool.query('SELECT COUNT(*) as count FROM inventory_stock WHERE location_id = ?', [id]);
        if (stockCountRows[0].count > 0) {
            req.flash('error', 'Tidak dapat menghapus lokasi ini karena masih ada stok produk di dalamnya.');
            return res.redirect('/admin/locations');
        }

        await Location.delete(id);
        req.flash('message', 'Lokasi berhasil dihapus!'); // Set message via flash
        res.redirect('/admin/locations'); // Redirect bersih
    } catch (err) {
        console.error('Error deleting location:', err);
        req.flash('error', 'Gagal menghapus lokasi. Terjadi kesalahan server atau ada data terkait.'); // Set error via flash
        res.redirect('/admin/locations');
    }
};
```

Perubahan dan Poin Penting:

    Penggunaan req.flash(): Semua req.query.message/req.query.error diganti dengan req.flash('message')/req.flash('error'). Ini adalah praktik terbaik untuk pesan singkat yang muncul sekali.

    Redirect vs Render: Konsistensi dalam pola Post/Redirect/Get (PRG). Setelah operasi POST (create, update, delete) atau validasi gagal yang memerlukan tampilan ulang form, saya menggunakan req.flash() diikuti res.redirect(). Ini mencegah masalah pengiriman ulang form.

    formData untuk Create Form: Di createLocation, ketika ada error validasi, saya menambahkan formData: req.body saat me-render ulang form. Ini agar data yang sudah diisi pengguna tidak hilang. Anda perlu menyesuaikan file create.ejs Anda untuk menggunakan ini (misalnya <input value="<%= formData ? formData.location_code : '' %>">).

    parentLocations dan locationTypes Re-fetching: Saat terjadi error pada createLocation atau updateLocation, saya memastikan parentLocations dan locationTypes diambil kembali sebelum me-render halaman, agar dropdown tetap terisi dengan benar.

    Validasi is_active: Menggunakan is_active === 'on' untuk checkbox, yang merupakan nilai standar saat form dikirim.

    Validasi deleteLocation:
        Saya mempertahankan pengecekan Location.getChildren(id). Anda harus memastikan model Location Anda memiliki fungsi getChildren ini. Jika belum, itu akan menjadi error berikutnya.

        Kueri SELECT COUNT(*) as count FROM inventory_stock WHERE location_id = ? yang Anda tambahkan sudah benar untuk memeriksa stok. Saya juga menambahkan stockCountRows[0].count untuk mengakses nilai count dengan benar.

### `wms-app/src/models/Location.js`

tidak perlu di revisi

### `wms-app/src/routes/inventoryStockRoutes.js`
```
// src/routes/inventoryStockRoutes.js
const express = require('express');
const router = express.Router();
const inventoryStockController = require('../controllers/inventoryStockController'); // Kita akan buat ini
const { authenticateToken, authorizeRoles } = require('../middleware/authMiddleware');

// Middleware untuk melindungi semua rute stok inventaris
// Sesuaikan peran yang diizinkan (misalnya admin, gudang, ppic)
router.use(authenticateToken, authorizeRoles('admin', 'gudang', 'ppic'));

// Rute untuk menampilkan daftar stok inventaris
router.get('/', inventoryStockController.listInventoryStock);

// Rute untuk menampilkan form tambah stok baru (e.g., in/out, initial stock)
// Ini mungkin akan lebih kompleks nanti, melibatkan Product ID dan Location ID
// Untuk sementara, kita buat yang sederhana
router.get('/create', authorizeRoles('admin', 'gudang'), inventoryStockController.showCreateInventoryStockForm);

// Rute untuk memproses penambahan stok baru
router.post('/create', authorizeRoles('admin', 'gudang'), inventoryStockController.createInventoryStock);

// Rute untuk menampilkan detail stok berdasarkan ID (opsional)
// router.get('/:id', inventoryStockController.getInventoryStockById);

// Rute untuk menampilkan form edit stok (misalnya koreksi stok)
router.get('/edit/:id', authorizeRoles('admin', 'gudang'), inventoryStockController.showEditInventoryStockForm);

// Rute untuk memproses pembaruan stok
router.post('/edit/:id', authorizeRoles('admin', 'gudang'), inventoryStockController.updateInventoryStock);

// Rute untuk menghapus entri stok (hati-hati dengan ini, biasanya tidak direkomendasikan langsung dihapus)
// Lebih baik menggunakan status atau mencatat pergerakan stok
router.post('/delete/:id', authorizeRoles('admin', 'gudang'), inventoryStockController.deleteInventoryStock);


module.exports = router;
```

### `wms-app/src/controllers/inventoryStockController.js`

Berikut adalah versi revisi dari src/controllers/inventoryStockController.js Anda yang menggunakan connect-flash dan penyesuaian lainnya:
```
// src/controllers/inventoryStockController.js
const InventoryStock = require('../models/InventoryStock');
const Product = require('../models/Product');
const Location = require('../models/Location');

// Fungsi bantu untuk memperbarui total stok di tabel products
async function updateProductCurrentStock(productId) {
    try {
        const totalStock = await InventoryStock.getTotalStockByProductId(productId);
        await Product.updateCurrentStock(productId, totalStock);
    } catch (err) {
        console.error(`Error updating current stock for product ${productId}:`, err);
        // Mungkin log ini, tapi jangan sampai crash aplikasi
    }
}

// Fungsi untuk menampilkan daftar stok inventaris
exports.listInventoryStocks = async (req, res) => {
    try {
        // Gabungkan data stok dengan nama produk dan lokasi (gunakan method getAllWithProductAndLocation)
        const stocks = await InventoryStock.getAllWithProductAndLocation(); // Harusnya ada di model
        const message = req.flash('message'); // Ambil pesan sukses dari connect-flash
        const error = req.flash('error');     // Ambil pesan error dari connect-flash

        res.render('admin/inventory_stocks/list', { // Render view list.ejs di folder admin/inventory_stocks
            stocks,
            user: req.user,
            message: message.length ? message : null, // connect-flash mengembalikan array
            error: error.length ? error : null        // connect-flash mengembalikan array
        });
    } catch (err) {
        console.error('Error fetching inventory stocks:', err);
        req.flash('error', 'Gagal mengambil data stok inventaris.'); // Set error via flash
        res.redirect('/admin/dashboard'); // Redirect ke dashboard admin jika error fatal
    }
};

// Fungsi untuk menampilkan form tambah stok baru (barang masuk)
exports.showAddStockForm = async (req, res) => {
    try {
        const products = await Product.getAll(); // Ambil semua produk untuk dropdown
        const locations = await Location.getAll(); // Ambil semua lokasi untuk dropdown

        res.render('admin/inventory_stocks/add', {
            user: req.user,
            products: products,
            locations: locations,
            error: req.flash('error').length ? req.flash('error') : null, // Ambil error dari flash
            formData: null // Untuk mengisi ulang form jika ada error validasi
        });
    } catch (err) {
        console.error('Error showing add stock form:', err);
        req.flash('error', 'Gagal menampilkan form penambahan stok.'); // Set error via flash
        res.redirect('/admin/dashboard');
    }
};

// Fungsi untuk memproses penambahan stok baru
exports.addStock = async (req, res) => {
    const { product_id, location_id, quantity, batch_number, expiry_date } = req.body;
    const parsedQuantity = parseInt(quantity);

    if (!product_id || !location_id || isNaN(parsedQuantity) || parsedQuantity <= 0) {
        req.flash('error', 'Produk, Lokasi, dan Kuantitas harus diisi (kuantitas harus lebih dari 0).');
        return res.render('admin/inventory_stocks/add', {
            user: req.user,
            products: await Product.getAll(),
            locations: await Location.getAll(),
            error: req.flash('error').length ? req.flash('error') : null,
            formData: req.body // Kirim kembali data form yang diisi sebelumnya
        });
    }

    try {
        let createdOrUpdated = false;
        // Cek apakah sudah ada stok produk yang sama di lokasi yang sama dengan batch_number yang sama
        let existingStocks = await InventoryStock.findByProductAndLocation(product_id, location_id);

        let foundExistingBatch = false;
        if (existingStocks && existingStocks.length > 0) {
            for (let stock of existingStocks) {
                // Periksa batch_number jika ada
                if (batch_number && stock.batch_number === batch_number) {
                    await InventoryStock.updateQuantity(stock.id, stock.quantity + parsedQuantity);
                    foundExistingBatch = true;
                    createdOrUpdated = true;
                    break;
                }
                // Jika batch_number tidak ada (atau null), dan hanya ada satu entry stok untuk produk/lokasi ini tanpa batch_number
                else if (!batch_number && !stock.batch_number && existingStocks.length === 1) {
                    await InventoryStock.updateQuantity(stock.id, stock.quantity + parsedQuantity);
                    foundExistingBatch = true;
                    createdOrUpdated = true;
                    break;
                }
            }
        }

        if (!foundExistingBatch) {
            // Jika tidak ditemukan batch yang sama atau belum ada stok di lokasi ini
            await InventoryStock.create({
                product_id: parseInt(product_id),
                location_id: parseInt(location_id),
                quantity: parsedQuantity,
                batch_number: batch_number || null,
                expiry_date: expiry_date || null
            });
            createdOrUpdated = true;
        }

        if (createdOrUpdated) {
            // Update total stok produk di tabel products
            await updateProductCurrentStock(product_id);
            req.flash('message', 'Stok berhasil ditambahkan!'); // Set message via flash
        } else {
            // Ini seharusnya tidak terjadi jika logika di atas benar, tapi sebagai fallback
            req.flash('error', 'Operasi penambahan stok tidak menghasilkan perubahan.');
        }

        res.redirect('/admin/inventory_stocks'); // Redirect bersih
    } catch (err) {
        console.error('Error adding stock:', err);
        req.flash('error', 'Gagal menambahkan stok. Terjadi kesalahan server.'); // Set error via flash
        res.render('admin/inventory_stocks/add', {
            user: req.user,
            products: await Product.getAll(),
            locations: await Location.getAll(),
            error: req.flash('error').length ? req.flash('error') : null,
            formData: req.body
        });
    }
};

// Fungsi untuk menampilkan form edit stok (untuk update kuantitas atau keluar stok)
exports.showEditStockForm = async (req, res) => {
    try {
        const stockId = req.params.id;
        const stock = await InventoryStock.findById(stockId);
        if (!stock) {
            req.flash('error', 'Catatan stok tidak ditemukan.'); // Set error via flash
            return res.redirect('/admin/inventory_stocks'); // Redirect jika tidak ditemukan
        }

        // Ambil data produk dan lokasi untuk ditampilkan
        const product = await Product.findById(stock.product_id);
        const location = await Location.findById(stock.location_id);

        res.render('admin/inventory_stocks/edit', {
            stock: stock,
            user: req.user,
            product: product,   // Kirim data produk
            location: location, // Kirim data lokasi
            error: req.flash('error').length ? req.flash('error') : null // Ambil error dari flash
        });
    } catch (err) {
        console.error('Error showing edit stock form:', err);
        req.flash('error', 'Gagal menampilkan form edit stok.'); // Set error via flash
        res.redirect('/admin/inventory_stocks');
    }
};

// Fungsi untuk memproses update stok (termasuk keluar stok)
exports.updateStock = async (req, res) => {
    const stockId = req.params.id;
    const { quantity } = req.body; // Kuantitas baru yang diinginkan
    const parsedQuantity = parseInt(quantity);

    if (isNaN(parsedQuantity) || parsedQuantity < 0) { // Kuantitas tidak boleh negatif
        req.flash('error', 'Kuantitas harus diisi dan tidak boleh negatif.');
        return res.redirect(`/admin/inventory_stocks/edit/${stockId}`); // Redirect dengan flash error
    }

    try {
        const currentStock = await InventoryStock.findById(stockId);
        if (!currentStock) {
            req.flash('error', 'Catatan stok tidak ditemukan.');
            return res.redirect('/admin/inventory_stocks');
        }

        // Cek jika kuantitas akan menjadi negatif (untuk outbound/pengurangan)
        // Logika ini harusnya lebih kompleks dengan transaction type (in/out)
        // Untuk update langsung seperti ini, pastikan tidak mengurangi melebihi yang ada
        // if (parsedQuantity < currentStock.quantity && parsedQuantity < 0) { // Contoh sederhana
        //    req.flash('error', 'Kuantitas tidak bisa lebih kecil dari 0.');
        //    return res.redirect(`/admin/inventory_stocks/edit/${stockId}`);
        // }


        // Update kuantitas di tabel inventory_stock
        await InventoryStock.updateQuantity(stockId, parsedQuantity); // Memerlukan updateQuantity di model

        // Update total stok produk di tabel products
        await updateProductCurrentStock(currentStock.product_id);

        req.flash('message', 'Stok berhasil diperbarui!'); // Set message via flash
        res.redirect('/admin/inventory_stocks'); // Redirect bersih
    } catch (err) {
        console.error('Error updating stock:', err);
        req.flash('error', 'Gagal memperbarui stok. Terjadi kesalahan server.'); // Set error via flash
        res.redirect(`/admin/inventory_stocks/edit/${stockId}`); // Redirect dengan flash error
    }
};

// Fungsi untuk memproses penghapusan catatan stok
exports.deleteStock = async (req, res) => {
    const { id } = req.params;
    try {
        const stockToDelete = await InventoryStock.findById(id);
        if (!stockToDelete) {
            req.flash('error', 'Catatan stok tidak ditemukan.');
            return res.redirect('/admin/inventory_stocks');
        }

        await InventoryStock.delete(id);

        // Update total stok produk di tabel products setelah penghapusan
        await updateProductCurrentStock(stockToDelete.product_id);

        req.flash('message', 'Catatan stok berhasil dihapus!'); // Set message via flash
        res.redirect('/admin/inventory_stocks'); // Redirect bersih
    } catch (err) {
        console.error('Error deleting stock:', err);
        req.flash('error', 'Gagal menghapus catatan stok. Terjadi kesalahan server.'); // Set error via flash
        res.redirect('/admin/inventory_stocks');
    }
};
```

### Revisi `wms-app/src/routes/inventoryStockRoutes.js`
```
// src/routes/inventoryStockRoutes.js
const express = require('express');
const router = express.Router();
const inventoryStockController = require('../controllers/inventoryStockController');
const { authenticateToken, authorizeRoles } = require('../middleware/authMiddleware');

// Middleware untuk melindungi semua rute stok inventaris
// Sesuaikan peran yang diizinkan (misalnya admin, gudang, ppic)
router.use(authenticateToken, authorizeRoles('admin', 'gudang', 'ppic'));

// Rute untuk menampilkan daftar stok inventaris
router.get('/', inventoryStockController.listInventoryStocks); // <-- INI YANG DIUBAH! Disesuaikan dengan controller

// Rute untuk menampilkan form tambah stok baru
router.get('/add', authorizeRoles('admin', 'gudang'), inventoryStockController.showAddStockForm); // Disesuaikan dengan controller

// Rute untuk memproses penambahan stok baru
router.post('/add', authorizeRoles('admin', 'gudang'), inventoryStockController.addStock); // Disesuaikan dengan controller

// Rute untuk menampilkan form edit stok
router.get('/edit/:id', authorizeRoles('admin', 'gudang'), inventoryStockController.showEditStockForm); // Disesuaikan dengan controller

// Rute untuk memproses pembaruan stok
router.post('/edit/:id', authorizeRoles('admin', 'gudang'), inventoryStockController.updateStock); // Disesuaikan dengan controller

// Rute untuk menghapus entri stok
router.post('/delete/:id', authorizeRoles('admin', 'gudang'), inventoryStockController.deleteStock); // Disesuaikan dengan controller

module.exports = router;
```

### C. Modul Tambahan & Peningkatan (Enhancements)

Ini akan membuat aplikasi lebih canggih dan mudah digunakan.

    1. Notifikasi & Peringatan:

      -Stok Minimum/Maksimum: Peringatan ketika stok suatu item mencapai batas minimum atau melebihi batas maksimum.
      
      -Kadaluarsa Produk: Notifikasi untuk produk yang mendekati tanggal kadaluarsa.

      -Aktivitas Penting: Notifikasi untuk penerimaan/pengiriman besar.

    2. Laporan & Analisis:

      -Laporan Stok: Laporan stok saat ini, stok masuk/keluar harian/mingguan/bulanan.

      -Laporan Pergerakan Barang: Analisis pergerakan barang.

      -Laporan Kinerja: Metrik seperti picking accuracy, receiving time, dll.
      
      -Dashboard yang Lebih Dinamis: Visualisasi data stok, pergerakan, dan kinerja.

    3. Integrasi Barcode/QR Code:

      -Mendukung scanning barcode/QR code untuk picking, receiving, stock count untuk mempercepat proses dan mengurangi error.

    4. Audit Trail:

      -Mencatat setiap perubahan penting dalam sistem (siapa yang melakukan, kapan, perubahan apa) untuk auditing dan keamanan.

    5. Manajemen Pemasok & Pelanggan:

      -CRUD Pemasok/Pelanggan: Menyimpan informasi kontak dan detail lainnya.

    6. Peningkatan UI/UX:

      -Menggunakan framework CSS (misalnya, Bootstrap, Tailwind CSS) untuk tampilan yang lebih modern dan responsif.

      -Peningkatan navigasi dan pengalaman pengguna secara keseluruhan.

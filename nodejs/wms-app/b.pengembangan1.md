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

### Mari kita buat sebuah API sederhana menggunakan Express.js yang akan mengelola data hotel/kamar, mirip dengan aplikasi UTS-mu. Kita akan membuat endpoint (titik akhir) untuk:

1. Mengambil daftar kamar.
2. Mengambil detail satu kamar.
3. Membuat kamar baru (simulasi).


### 1.Persiapan Lingkungan
Buat Folder Proyek Baru:
Buka terminal Debian 12 kamu.
```
mkdir hotel-api
cd hotel-api
```

### 2.Inisialisasi Proyek Node.js:
Ini akan membuat file `package.json` yang menyimpan metadata proyek dan daftar dependency.
```
npm init -y
```

### 3.Instal Express.js:
```
npm install express
```

### 4.Buat File Server:
Buat file baru bernama `app.js` (atau `server.js`) di dalam folder `hotel-api`

Kode API Sederhana (app.js)

Salin kode berikut dan tempelkan ke dalam file `app.js`:
```
// Import Express
const express = require('express');
const app = express();
const port = 3000; // Port yang akan digunakan untuk menjalankan server

// Middleware untuk mengizinkan server membaca body dari permintaan JSON
app.use(express.json());

// Data dummy (simulasi database)
let rooms = [
    { id: 'S123', name: 'Superior Room', price: 150000, type: 'Superior' },
    { id: 'D123', name: 'Deluxe Room', price: 250000, type: 'Deluxe' },
    { id: 'S321', name: 'Suite Room', price: 400000, type: 'Suite' },
    { id: 'D456', name: 'Deluxe Twin', price: 270000, type: 'Deluxe' }
];

// 1. Endpoint: GET /api/rooms
// Untuk mengambil semua daftar kamar
app.get('/api/rooms', (req, res) => {
    console.log('GET /api/rooms request received');
    res.json(rooms); // Mengirim data rooms sebagai JSON
});

// 2. Endpoint: GET /api/rooms/:id
// Untuk mengambil detail satu kamar berdasarkan ID
app.get('/api/rooms/:id', (req, res) => {
    const roomId = req.params.id; // Mengambil ID dari parameter URL
    const room = rooms.find(r => r.id === roomId); // Mencari kamar berdasarkan ID

    if (room) {
        console.log(`GET /api/rooms/${roomId} request received`);
        res.json(room); // Mengirim data kamar yang ditemukan
    } else {
        console.log(`GET /api/rooms/${roomId} - Room not found`);
        // Mengirim status 404 Not Found jika kamar tidak ditemukan
        res.status(404).json({ message: 'Room not found' });
    }
});

// 3. Endpoint: POST /api/rooms
// Untuk membuat kamar baru (simulasi)
app.post('/api/rooms', (req, res) => {
    const newRoom = req.body; // Mengambil data dari body permintaan (berbentuk JSON)

    // Validasi sederhana: Pastikan ada ID dan nama
    if (!newRoom.id || !newRoom.name || !newRoom.price || !newRoom.type) {
        console.log('POST /api/rooms - Invalid data received');
        // Mengirim status 400 Bad Request jika data tidak lengkap
        return res.status(400).json({ message: 'ID, name, price, and type are required' });
    }

    // Cek apakah ID sudah ada
    if (rooms.find(r => r.id === newRoom.id)) {
        console.log(`POST /api/rooms - Room with ID ${newRoom.id} already exists`);
        return res.status(409).json({ message: 'Room with this ID already exists' }); // 409 Conflict
    }

    rooms.push(newRoom); // Menambahkan kamar baru ke array (simulasi penyimpanan)
    console.log('POST /api/rooms request received. New room added:', newRoom);
    // Mengirim status 201 Created dan data kamar yang baru dibuat
    res.status(201).json({ message: 'Room created successfully', room: newRoom });
});

// Jalankan server
app.listen(port, () => {
    console.log(`Server API hotel berjalan di http://localhost:${port}`);
    console.log('Tekan Ctrl+C untuk menghentikan server.');
});
```

Penjelasan Kode app.js:
1.`const express = require('express');`: Mengimpor modul Express.
2.`const app = express();`: Membuat instansi aplikasi Express.
3.`const port = 3000;`: Mendefinisikan port tempat server akan berjalan.
4.`app.use(express.json());`: Ini adalah middleware penting. Ini memberitahu Express untuk otomatis menguraikan (parse) body permintaan yang masuk jika formatnya JSON. Tanpa ini, `req.body` akan undefined saat Anda mengirim data POST.
5.`let rooms = [...]`: Ini adalah data dummy kita. Dalam aplikasi real-world, ini akan diganti dengan koneksi ke database (misalnya MongoDB, PostgreSQL, MySQL). Karena ini hanya simulasi, data akan hilang setiap kali server dihentikan.
6.`app.get('/api/rooms', ...)`:
  1.Mendefinisikan endpoint GET pada jalur `/api/rooms`.
  2.Ketika ada permintaan GET ke jalur ini, callback function akan dieksekusi.
  3.`res.json(rooms)`: Mengirim array rooms sebagai respons JSON.
7.`app.get('/api/rooms/:id', ...)`:
  1.Mendefinisikan endpoint GET untuk detail kamar. :`id` adalah parameter yang bisa diakses melalui `req.params.id`.
  2.Menggunakan `rooms.find()` untuk mencari kamar.
  3.Jika ditemukan, mengirim data kamar. Jika tidak, mengirim status `404 Not Found` dan pesan error.
8.`app.post('/api/rooms', ...)`:
  1.Mendefinisikan endpoint POST pada jalur `/api/rooms`.
  2.`const newRoom = req.body;`: Mengambil data yang dikirim oleh klien dari body permintaan.
  3.Melakukan validasi sederhana untuk memastikan data lengkap dan ID belum ada.
  4.`rooms.push(newRoom);`: Menambahkan kamar baru ke array `rooms`.
  5.`res.status(201).json(...)`: Mengirim status `201 Created` (standar HTTP untuk sumber daya baru berhasil dibuat) dan data kamar yang baru.
9.`app.listen(port, ...)`: Menjalankan server Express pada port yang ditentukan.

### Cara Menjalankan API Anda

Buka Terminal di folder `hotel-api` Anda.

Jalankan server `Node.js`:
```
node app
```
Anda akan melihat pesan: `Server API hotel berjalan di http://localhost:3000`


## Cara Menguji API Anda

Sekarang, buka terminal baru (biarkan server yang pertama tetap berjalan di terminal lain) atau gunakan alat seperti curl.

1. Menguji GET Semua Kamar:
```
curl http://localhost:3000/api/rooms
```

Output yang diharapkan: Akan menampilkan array JSON berisi semua data kamar yang ada di `rooms`.

2. Menguji GET Satu Kamar (dengan ID yang ada):
```
curl http://localhost:3000/api/rooms/S123
```

Output yang diharapkan: Akan menampilkan objek JSON untuk kamar dengan ID `S123`.

3. Menguji GET Satu Kamar (dengan ID yang tidak ada):
```
curl http://localhost:3000/api/rooms/X999
```

Output yang diharapkan: Akan menampilkan respons error JSON dengan status 404 Not Found.

4. Menguji POST (Membuat Kamar Baru):
```
curl -X POST -H "Content-Type: application/json" -d '{"id": "M789", "name": "Standard Max", "price": 100000, "type": "Standard"}' http://localhost:3000/api/rooms
```

Output yang diharapkan: Akan menampilkan respons JSON dengan status `201 Created` dan detail kamar yang baru dibuat.
Jika Anda menjalankan `curl http://localhost:3000/api/rooms` lagi setelah ini, Anda akan melihat `M789` ditambahkan ke daftar!

5. Menguji POST (Data Tidak Lengkap / ID Duplikat):
```
# Data tidak lengkap
curl -X POST -H "Content-Type: application/json" -d '{"name": "Missing ID"}' http://localhost:3000/api/rooms

# ID duplikat (setelah berhasil membuat M789)
curl -X POST -H "Content-Type: application/json" -d '{"id": "M789", "name": "Another Room", "price": 100000, "type": "Standard"}' http://localhost:3000/api/rooms
```

Output yang diharapkan: Akan menampilkan respons error JSON dengan status `400 Bad Request` atau `409 Conflict`.   

Langkah selanjutnya yang bisa Anda pelajari:

Database: Mengganti array rooms dengan database sungguhan (misalnya MongoDB dengan Mongoose, atau PostgreSQL dengan Sequelize/Knex.js) agar data persisten.

PUT dan DELETE: Menambahkan endpoint untuk memperbarui dan menghapus data.

Autentikasi & Otorisasi: Menambahkan sistem login dan middleware untuk melindungi endpoint API tertentu.

Input Lebih Kompleks: Menggunakan library validasi (misalnya express-validator) untuk memeriksa input dari klien dengan lebih cermat.

Pengujian Otomatis: Menulis unit test dan integration test untuk API Anda.

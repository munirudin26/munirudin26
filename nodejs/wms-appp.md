## perplexity

```
wms-app/
├── node_modules/               # Dependencies Node.js
├── src/
│   ├── config/                 # Konfigurasi aplikasi
│   │   └── database.js         # Pool koneksi database MySQL/MariaDB
│   ├── controllers/            # Logika bisnis dan pengolahan request
│   │   ├── authController.js   # Login, logout, autentikasi
│   │   ├── userController.js   # Manajemen user/admin
│   │   ├── productController.js
│   │   ├── locationController.js
│   │   ├── inventoryStockController.js
│   │   └── receivingController.js
│   ├── errors/                 # Custom error classes
│   │   └── DatabaseError.js
│   ├── middleware/             # Middleware Express (auth, otorisasi)
│   │   └── authMiddleware.js
│   ├── models/                 # Model untuk query ke database
│   │   ├── User.js
│   │   ├── Product.js
│   │   ├── Location.js
│   │   ├── InventoryStock.js
│   │   ├── PurchaseOrder.js
│   │   └── ReceivingRecord.js
│   ├── routes/                 # Definisi endpoint API
│   │   ├── authRoutes.js
│   │   ├── adminRoutes.js
│   │   ├── userRoutes.js
│   │   ├── productRoutes.js
│   │   ├── locationRoutes.js
│   │   ├── inventoryStockRoutes.js
│   │   └── receivingRoutes.js
│   ├── utils/                  # Utility functions, logger, dll
│   │   └── logger.js
│   └── app.js                  # Entrypoint aplikasi Express
├── views/                      # Template EJS untuk frontend
│   ├── auth/
│   │   └── login.ejs
│   ├── admin/
│   │   └── users/
│   │       ├── list.ejs
│   │       ├── create.ejs
│   │       └── edit.ejs
│   │   └── change-password.ejs
│   ├── dashboard/
│   │   ├── admin.ejs
│   │   ├── ppic.ejs
│   │   ├── packing.ejs
│   │   ├── mixing.ejs
│   │   ├── milling.ejs
│   │   ├── qc.ejs
│   │   └── gudang.ejs
│   └── error.ejs
├── .env                        # Environment variables (private)
├── .env.example                # Contoh template .env
├── package.json                # Metadata dan dependencies project
├── package-lock.json           # Lock file dependencies
└── hash_password.js            # Utilitas untuk generate hash password (opsional)

```

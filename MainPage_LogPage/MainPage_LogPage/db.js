const mysql = require('mysql2');

const db = mysql.createConnection({
  host: 'localhost',
  user: 'root',
  password: '',        // ใส่รหัสผ่านถ้ามี
  database: 'userdb'
});

db.connect(err => {
  if (err) console.error('❌ Database connection failed:', err);
  else console.log('✅ Connected to MySQL (userdb)');
});

module.exports = db;

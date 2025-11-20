var express = require('express');
var router = express.Router();
const bcrypt = require('bcryptjs');
const db = require('../db');

// หน้าแรก -> ไปหน้า login
router.get('/', (req, res) => {
  res.redirect('/login');
});

// แสดงหน้า login
router.get('/login', (req, res) => {
  res.render('indexlogin', { title: 'Login', error: null });
});

// รับ login
router.post('/login', (req, res) => {
  const { email, password } = req.body;
  if (!email || !password) {
    return res.status(400).render('indexlogin', { title: 'Login', error: 'กรอกอีเมลและรหัสผ่าน' });
  }

  // หา user จากฐานข้อมูล
 const sql = 'SELECT id, email, password FROM users WHERE email = ? LIMIT 1';
  db.query(sql, [email], (err, results) => {
    if (err) {
      console.error('DB error:', err);
      return res.status(500).render('indexlogin', { title: 'Login', error: 'เซิร์ฟเวอร์มีปัญหา ลองใหม่อีกครั้ง' });
    }
    if (results.length === 0) {
      return res.status(401).render('indexlogin', { title: 'Login', error: 'อีเมลหรือรหัสผ่านไม่ถูกต้อง' });
    }

    const user = results[0];
    // เปรียบเทียบรหัสผ่าน
    bcrypt.compare(password, user.password, (err, ok) => {
      if (err) {
        console.error(err);
        return res.status(500).render('indexlogin', { title: 'Login', error: 'เกิดข้อผิดพลาด' });
      }
      if (!ok) {
        return res.status(401).render('indexlogin', { title: 'Login', error: 'อีเมลหรือรหัสผ่านไม่ถูกต้อง' });
      }

      // ตั้ง session แล้วไปหน้า main
      req.session.user = { id: user.id, email: user.email };
      res.redirect('/main');
    });
  });
});

// ออกจากระบบ
router.get('/logout', (req, res) => {
  req.session.destroy(() => {
    res.clearCookie('connect.sid');
    res.redirect('/login');
  });
});

// หน้า Register (ถ้ามีฟอร์มสมัคร)
router.get('/register', (req, res) => {
  res.render('indexRegister', { title: 'Register' });
});

// ตัวอย่างรับสมัคร (ถ้าคุณต้องการ)
router.post('/register', async (req, res) => {
  const { name, email, password } = req.body;
  if (!name || !email || !password) {
    return res.status(400).render('indexRegister', { title: 'Register', error: 'กรอกข้อมูลให้ครบ' });
  }
  const hash = await bcrypt.hash(password, 10);
  db.query('INSERT INTO users (name, email, password) VALUES (?,?,?)', [name, email, hash], (err) => {
    if (err) {
      console.error(err);
      return res.status(500).render('indexRegister', { title: 'Register', error: 'สมัครไม่ได้ กรุณาลองใหม่' });
    }
    res.redirect('/login');
  });
});

// หน้า main (ต้อง login ก่อน)
router.get('/main', (req, res) => {
  if (!req.session || !req.session.user) {
    return res.redirect('/login');
  }
  res.render('indexmain', {
    title: 'Main Page',
    user: req.session.user
  });
});

// ตัวอย่างหน้าอื่นๆ
router.get('/setting', (req, res) => {
  if (!req.session || !req.session.user) return res.redirect('/login');
  res.render('indexSetting', { title: 'Setting', user: req.session.user });
});

router.get('/status', (req, res) => {
  if (!req.session || !req.session.user) return res.redirect('/login');
  res.render('indexStatus', { title: 'Status', user: req.session.user });
});

module.exports = router;

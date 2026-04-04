#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>

typedef unsigned long long uint64;

// Fungsi mencari PBB (FPB / GCD)
uint64 gcd(uint64 a, uint64 b) {
    uint64 t;
    while(1) {
        t = a % b;
        if(t == 0) return b;
        a = b;
        b = t;
    }
}

// Mengecek bilangan prima
bool isPrime(uint64 n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (uint64 i = 5; i * i <= n; i = i + 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    return true;
}

// Membuat bilangan prima acak
uint64 generatePrime(uint64 minVal, uint64 maxVal) {
    while (true) {
        uint64 candidate = minVal + rand() % (maxVal - minVal + 1);
        if (isPrime(candidate)) return candidate;
    }
}

// Modular Exponentiation (base^exp mod modulus)
uint64 modPow(uint64 base, uint64 exp, uint64 mod) {
    uint64 res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1)
            res = (res * base) % mod;
        exp = exp / 2;
        base = (base * base) % mod;
    }
    return res;
}

// Modular Inverse mengembalikan nilai x dimana (e * x) % phi == 1
uint64 modInverse(uint64 e, uint64 phi) {
    long long t = 0, newt = 1;
    long long r = phi, newr = e;

    while (newr != 0) {
        long long quotient = r / newr;
        long long temp_t = t - quotient * newt;
        t = newt;
        newt = temp_t;

        long long temp_r = r - quotient * newr;
        r = newr;
        newr = temp_r;
    }

    if (r > 1) return 0; // Tidak konvergen (bukan relatif prima)
    if (t < 0) t = t + phi;

    return (uint64)t;
}

// Algoritma Pembuatan Kunci RSA
void generateRSAKeys(uint64& e, uint64& d, uint64& n) {
    srand((unsigned)time(0));
    // Kita gunakan nilai kecil untuk mencegah overflow dari uint64 saat perkalian (base*base) % mod
    // Secara teoritis batas amannya adalah n < sqrt(UINT64_MAX), jadi sekitar ~3 milyar.
    uint64 p = generatePrime(100, 500); 
    uint64 q = generatePrime(100, 500);
    while (p == q) {
        q = generatePrime(100, 500);
    }
    
    n = p * q;
    uint64 phi = (p - 1) * (q - 1);
    
    e = 2;
    while (e < phi) {
        if (gcd(e, phi) == 1) break;
        e++;
    }
    
    int k = 1;
    while (1) {
        if (((phi * k) + 1) % e == 0) {
            d = ((phi * k) + 1) / e;
            break;
        }
        k++;
    }
}

// Enkripsi Cipher per karakter string
std::vector<uint64> encryptRSA(const std::string& message, uint64 e, uint64 n) {
    std::vector<uint64> cipher;
    for (char c : message) {
        cipher.push_back(modPow((uint64)c, e, n));
    }
    return cipher;
}

// Dekripsi kembalikan ke string asli
std::string decryptRSA(const std::vector<uint64>& cipher, uint64 d, uint64 n) {
    std::string decrypted_msg = "";
    for (uint64 c : cipher) {
        decrypted_msg += (char)modPow(c, d, n);
    }
    return decrypted_msg;
}

// Utilitas konversi uint64 array menjadi spasi bilangan berurut
std::string cipherToString(const std::vector<uint64>& cipher) {
    std::ostringstream oss;
    for (size_t i = 0; i < cipher.size(); ++i) {
        if (i != 0) oss << " ";
        oss << cipher[i];
    }
    return oss.str();
}

// Utilitas kembalikan spasi bilangan array menjadi uint64 array
std::vector<uint64> stringToCipher(const std::string& str) {
    std::vector<uint64> cipher;
    std::istringstream iss(str);
    uint64 num;
    while (iss >> num) {
        cipher.push_back(num);
    }
    return cipher;
}

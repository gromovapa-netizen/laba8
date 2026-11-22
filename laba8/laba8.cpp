#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <exception>

class HashTableException : public std::exception {
protected:
    std::string message;
public:
    explicit HashTableException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class VolumeException : public HashTableException {
public:
    explicit VolumeException(const std::string& msg) : HashTableException(msg) {}
};

class OverflowException : public VolumeException {
public:
    OverflowException() : VolumeException("Ошибка объема: достигнут максимальный размер таблицы (переполнение сверху).") {}
};

class UnderflowException : public VolumeException {
public:
    UnderflowException() : VolumeException("Ошибка объема: попытка удаления из пустой таблицы (переполнение снизу).") {}
};

class KeyException : public HashTableException {
public:
    explicit KeyException(const std::string& msg) : HashTableException(msg) {}
};

class KeyNotFoundException : public KeyException {
public:
    explicit KeyNotFoundException(const std::string& key)
        : KeyException("Ошибка ключа: ключ '" + key + "' не найден в таблице.") {
    }
};

class DuplicateKeyException : public KeyException {
public:
    explicit DuplicateKeyException(const std::string& key)
        : KeyException("Ошибка ключа: ключ '" + key + "' уже существует в таблице.") {
    }
};

class EmptyKeyException : public KeyException {
public:
    EmptyKeyException() : KeyException("Ошибка ключа: ключ не может быть пустым.") {}
};

class ValueRangeException : public HashTableException {
public:
    explicit ValueRangeException(const std::string& msg) : HashTableException(msg) {}
};

class ValueTooShortException : public ValueRangeException {
public:
    ValueTooShortException(size_t len, size_t minLen)
        : ValueRangeException("Ошибка диапазона: длина значения (" + std::to_string(len) +
            ") меньше минимальной (" + std::to_string(minLen) + ").") {
    }
};

class ValueTooLongException : public ValueRangeException {
public:
    ValueTooLongException(size_t len, size_t maxLen)
        : ValueRangeException("Ошибка диапазона: длина значения (" + std::to_string(len) +
            ") превышает максимальную (" + std::to_string(maxLen) + ").") {
    }
};

class HashTable {
private:
    struct Entry {
        std::string key;
        std::string value;
    };

    std::vector<std::list<Entry>> buckets;
    size_t maxCapacity;
    size_t currentSize;
    size_t minValLength;
    size_t maxValLength;

    size_t hash(const std::string& key) const {
        size_t hashVal = 0;
        for (char c : key) {
            hashVal = hashVal * 31 + static_cast<unsigned char>(c);
        }
        return hashVal % buckets.size();
    }

    void validateKey(const std::string& key) const {
        if (key.empty()) {
            throw EmptyKeyException();
        }
    }

    void validateValue(const std::string& value) const {
        if (value.length() < minValLength) {
            throw ValueTooShortException(value.length(), minValLength);
        }
        if (value.length() > maxValLength) {
            throw ValueTooLongException(value.length(), maxValLength);
        }
    }

public:
    HashTable(size_t capacity = 10, size_t minLen = 1, size_t maxLen = 50, size_t bucketCount = 7)
        : buckets(bucketCount),
        maxCapacity(capacity),
        currentSize(0),
        minValLength(minLen),
        maxValLength(maxLen) {
    }

    void insert(const std::string& key, const std::string& value) {
        validateKey(key);
        validateValue(value);

        if (currentSize >= maxCapacity) {
            throw OverflowException();
        }

        size_t index = hash(key);
        for (const auto& entry : buckets[index]) {
            if (entry.key == key) {
                throw DuplicateKeyException(key);
            }
        }

        buckets[index].push_back({ key, value });
        currentSize++;
    }

    std::string remove(const std::string& key) {
        if (currentSize == 0) {
            throw UnderflowException();
        }
        validateKey(key);

        size_t index = hash(key);
        auto& chain = buckets[index];
        for (auto it = chain.begin(); it != chain.end(); ++it) {
            if (it->key == key) {
                std::string extractedValue = it->value;
                chain.erase(it);
                currentSize--;
                return extractedValue;
            }
        }

        throw KeyNotFoundException(key);
    }

    bool find(const std::string& key, std::string& result) const {
        validateKey(key);
        size_t index = hash(key);
        for (const auto& entry : buckets[index]) {
            if (entry.key == key) {
                result = entry.value;
                return true;
            }
        }
        return false;
    }

    std::string& operator[](const std::string& key) {
        validateKey(key);
        size_t index = hash(key);
        for (auto& entry : buckets[index]) {
            if (entry.key == key) {
                return entry.value;
            }
        }
        throw KeyNotFoundException(key);
    }

    size_t size() const { return currentSize; }
};

int main() {
    setlocale(LC_ALL, "Russian");

    HashTable table(2, 3, 10);

    try {
        table.remove("user1");
    }
    catch (const UnderflowException& e) {
        std::cout << "[Перехват наследника] " << e.what() << "\n";
    }
    catch (const VolumeException& e) {
        std::cout << "[Перехват категории Volume] " << e.what() << "\n";
    }

    try {
        table.insert("user1", "Alex");
        table.insert("user2", "Hi");
    }
    catch (const ValueRangeException& e) {
        std::cout << "[Перехват категории ValueRange] " << e.what() << "\n";
    }

    try {
        table.insert("user2", "Dmitry");
        table.insert("user3", "Elena");
    }
    catch (const VolumeException& e) {
        std::cout << "[Перехват категории Volume] " << e.what() << "\n";
    }

    try {
        std::cout << "Значение user1: " << table["user1"] << "\n";
        std::cout << "Значение unknown: " << table["unknown"] << "\n";
    }
    catch (const KeyNotFoundException& e) {
        std::cout << "[Перехват наследника] " << e.what() << "\n";
    }
    catch (const KeyException& e) {
        std::cout << "[Перехват категории Key] " << e.what() << "\n";
    }
    catch (const HashTableException& e) {
        std::cout << "[Общий перехват] " << e.what() << "\n";
    }

    std::string foundVal;
    if (table.find("user2", foundVal)) {
        std::cout << "Найдено через find('user2'): " << foundVal << "\n";
    }

    std::cout << "Извлечено через remove('user1'): " << table.remove("user1") << "\n";
    std::cout << "Текущий размер таблицы: " << table.size() << "\n";

    return 0;
}

#include <iostream>
#include <string>
#include <random>
#include <atomic>
#include <mutex>

inline void is_nullptr(const void *ptr, const std::string &fun, const std::string &details)
{
    if (ptr == nullptr)
    {
        std::cerr << "DataBlob::" << fun << ": " << details << std::endl;
        exit(EXIT_FAILURE);
    }
}


enum DataType
{
    INT,
    DOUBLE,
    UNSIGNED_CHAR,
    SHORT,
    FLOAT,
    LONG
};


class ReferenceCounter
{
public:
    ReferenceCounter() : m_count(1) {}
    void incease();
    bool decrease();
    size_t getCount();

private:
    std::atomic_size_t m_count;
    std::mutex m_mutex;
};


class DataBlob
{
private:
    struct Data
    {
        void *data;
        ReferenceCounter *referenceCounter;
        size_t rows;
        size_t cols;
        size_t channels;
        DataType type;
        size_t type_size;
        size_t length;

        Data(size_t r, size_t c, size_t ch, DataType type);
        Data(size_t r, size_t c, size_t ch, DataType type, void *data);
        void set_size_of_type();
        ~Data();
        Data *clone();
    };

    bool data_dimension(const DataBlob &other) const;                                                                                                           // 判断Blob_Data尺寸，类型是否相等
    bool mul_dimension(const DataBlob &other) const;                                                                                                            // 判断是否能进行乘法运算
    void is_valid() const;                                                                                                                                      // 判断本类的数据是否合法
    bool is_part(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end) const;                         // 判断截取位置是否合法
    template <typename T>
    T *generate_random_array(size_t length, int start, int end);                                                                                                // 构造随机数据
    template <typename T>
    friend std::ostream &printData(std::ostream &os, const DataBlob blob);                                                                                      // 打印数据
    template <typename T>
    DataBlob add(const DataBlob &other) const;                                                                                                                  // 加法
    template <typename T> 
    DataBlob minus(const DataBlob &other) const;                                                                                                                // 减法
    template <typename T>
    DataBlob mul(const DataBlob &other) const;                                                                                                                  // 乘法
    template <typename T>
    DataBlob transposition() const;                                                                                                                             // 转置
    template <typename T>
    DataBlob get_PART(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end) const;                    // 获取矩阵的一部分
    template <typename T>
    void set_PART(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end, const DataBlob &other) const; // 设置矩阵的一部分
    void release();                                                                                                                                             // 释放内存

    Data *Blob_Data;

public:
    DataBlob(size_t r, size_t c, size_t ch, DataType type);                                 // 不含数据构造函数
    DataBlob(size_t r, size_t c, size_t ch, DataType type, void *data);                     // 含数据构造函数
    DataBlob(const DataBlob &other);                                                        // 拷贝构造函数
    DataBlob(DataType type, int start, int end, size_t r, size_t c, size_t ch);             // 随机数据构造函数
    ~DataBlob();                                                                            // 析构函数

    template <typename T>
    T *convertPtr(const void *ptr) const;                 // 转化指针类型
    template <typename T>
    T *convertPtr() const;                                // 转化本类的数据指针
    template <typename T>
    T at(size_t r, size_t c, size_t ch) const;            // 获取元素
    template <typename T>
    void set(size_t r, size_t c, size_t ch, T value);     // 设置元素
    template <typename T>
    void set_all(size_t r, size_t c, size_t ch, T value); // 设置元素(所有共享数据的类的数据均改变)

    DataBlob operator+(const DataBlob &other) const;                        // 重写+
    DataBlob operator-(const DataBlob &other) const;                        // 重写-
    DataBlob operator*(const DataBlob &other) const;                        // 重写*
    DataBlob operator~() const;                                             // 重写~ 转置
    DataBlob &operator=(const DataBlob &other);                             // 重写赋值运算符
    bool operator==(const DataBlob &other) const;                           // 重写==运算符
    bool operator!=(const DataBlob &other) const;                           // 重写!=运算符
    friend std::ostream &operator<<(std::ostream &os, const DataBlob blob); // 重写输出流

    DataBlob get_part(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end) const;                        // 获取矩阵的一部分
    void set_part_all(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end, const DataBlob &other) const; // 设置矩阵的一部分(所以共享数据的类的对象数据均改变)
    void set_part(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end, const DataBlob &other);           // 设置矩阵的一部分
    DataBlob clone() const;                                                                                                                                         // 对对象进行克隆
    size_t get_reference_count();                                                                                                                                   // 获取引用次数
};


void ReferenceCounter::incease()
{
    m_mutex.lock();
    ++m_count;
    m_mutex.unlock();
}

bool ReferenceCounter::decrease()
{
    m_mutex.lock();
    --m_count;
    bool result = (m_count == 0);
    m_mutex.unlock();
    return result;
}

size_t ReferenceCounter::getCount()
{
    m_mutex.lock();
    size_t count = m_count;
    m_mutex.unlock();
    return count;
}

void DataBlob::Data::set_size_of_type()
{
    switch (type)
    {
    case DataType::INT:
    {
        this->type_size = sizeof(int);
        break;
    }
    case DataType::DOUBLE:
    {
        this->type_size = sizeof(double);
        break;
    }
    case DataType::FLOAT:
    {
        this->type_size = sizeof(float);
        break;
    }
    case DataType::LONG:
    {
        this->type_size = sizeof(long);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        this->type_size = sizeof(unsigned char);
        break;
    }
    default:
        std::cerr << "Invalid Type" << std::endl;
    }
}

// Data: 不含数据构造函数
DataBlob::Data::Data(size_t r, size_t c, size_t ch, DataType type)
{
    rows = r;
    cols = c;
    channels = ch;
    this->type = type;
    this->set_size_of_type();
    length = r * c * ch;
    data = new char[length * type_size];
    is_nullptr(data, "Data:Construct", "fail to allocate memory for data.");
    referenceCounter = new ReferenceCounter();
}

// Data: 含数据构造函数
DataBlob::Data::Data(size_t r, size_t c, size_t ch, DataType type, void *data)
{
    rows = r;
    cols = c;
    channels = ch;
    this->type = type;
    this->set_size_of_type();
    length = r * c * ch;
    this->data = data;
    referenceCounter = new ReferenceCounter();
}

DataBlob::Data::~Data()
{
    delete[] (char *)data;
    delete referenceCounter;
}

DataBlob::Data *DataBlob::Data::clone()
{
    Data *newData = new Data(rows, cols, channels, type);
    is_nullptr(newData, "Data:clone", "fail to allocate memory for Data object.");
    memcpy(newData->data, data, rows * cols * channels * type_size);
    return newData;
}

// 不含数据的构造函数
DataBlob::DataBlob(size_t r, size_t c, size_t ch, DataType type)
{
    if (r <= 0 || c <= 0 || ch <= 0)
    {
        std::cerr << "DataBlob: dimensions must be positive!" << std::endl;
        exit(EXIT_FAILURE);
    }
    Blob_Data = new Data(r, c, ch, type);
    is_nullptr(Blob_Data, "Construct", "fail to allocate memory for new Data object.");
}

// 含数据的构造函数
DataBlob::DataBlob(size_t r, size_t c, size_t ch, DataType type, void *data)
{
    if (r <= 0 || c <= 0 || ch <= 0)
    {
        std::cerr << "DataBlob: dimensions must be positive!" << std::endl;
        exit(EXIT_FAILURE);
    }
    Blob_Data = new Data(r, c, ch, type, data);
    is_nullptr(Blob_Data, "Construct", "fail to allocate memory for new Data object.");
}

// 构造随机数据
DataBlob::DataBlob(DataType type, int start, int end, size_t r, size_t c, size_t ch)
{
    void *newdata;
    switch (type)
    {
    case DataType::INT:
    {
        newdata = generate_random_array<int>(r * c * ch, start, end);
        break;
    }
    case DataType::DOUBLE:
    {
        newdata = generate_random_array<double>(r * c * ch, start, end);
        break;
    }
    case DataType::FLOAT:
    {
        newdata = generate_random_array<float>(r * c * ch, start, end);
        break;
    }
    case DataType::SHORT:
    {
        newdata = generate_random_array<short>(r * c * ch, start, end);
        break;
    }
    case DataType::LONG:
    {
        newdata = generate_random_array<long>(r * c * ch, start, end);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        newdata = generate_random_array<unsigned char>(r * c * ch, start, end);
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type!");
    }
    if (r <= 0 || c <= 0 || ch <= 0)
    {
        std::cerr << "DataBlob: dimensions must be positive!" << std::endl;
        exit(EXIT_FAILURE);
    }
    Blob_Data = new Data(r, c, ch, type, newdata);
    is_nullptr(Blob_Data, "Construct", "fail to allocate memory for new Data object.");
}

// 拷贝构造函数
DataBlob::DataBlob(const DataBlob &other)
{
    Blob_Data = other.Blob_Data;
    Blob_Data->referenceCounter->incease();
    // std::cout << "The copy constructor is used" << std::endl;
}

// 析构函数
DataBlob::~DataBlob()
{
    release();
    // std::cout << "The destructor of DataBlob is used" << std::endl;
}

// 构造随机数据
template <typename T>
T *DataBlob::generate_random_array(size_t length, int start, int end)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(start, end);

    T *arr = new T[length];
    for (size_t i = 0; i < length; i++)
    {
        arr[i] =T(dis(gen));
    }

    return arr;
}

// 判断Blob_Data尺寸，类型是否相等
bool DataBlob::data_dimension(const DataBlob &other) const
{
    if (Blob_Data->rows != other.Blob_Data->rows || Blob_Data->cols != other.Blob_Data->cols ||
        Blob_Data->channels != other.Blob_Data->channels || Blob_Data->type != other.Blob_Data->type)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// 判断是否能进行乘法运算
bool DataBlob::mul_dimension(const DataBlob &other) const
{
    if (Blob_Data->cols != other.Blob_Data->rows || Blob_Data->channels != other.Blob_Data->channels ||
        Blob_Data->type != other.Blob_Data->type)
    {
        return false;
    }
    else
    {
        return true;
    }
}
void DataBlob::is_valid() const
{
    is_nullptr(Blob_Data, "is_valid()", "Blob_Data is null!");
    is_nullptr(Blob_Data->data, "is_valid()", "data is null!");
    is_nullptr(Blob_Data->referenceCounter, "is_valid", "referenceCounter is null!");
}

// 赋值运算符
DataBlob &DataBlob::operator=(const DataBlob &other)
{
    if (this != &other)
    {
        release();
        Blob_Data = other.Blob_Data;
        Blob_Data->referenceCounter->incease();
    }
    // std::cout << "The assignment operator is used" << std::endl;
    return *this;
}

bool DataBlob::operator==(const DataBlob &other) const
{
    is_valid();
    other.is_valid();
    if (!data_dimension(other))
    {
        return false;
    }
    return std::memcmp(Blob_Data->data, other.Blob_Data->data, Blob_Data->length * other.Blob_Data->type_size) == 0;
}

bool DataBlob::operator!=(const DataBlob &other) const
{
    return !(*this == other);
}

// 转化指针类型
template <typename T>
T *DataBlob::convertPtr(const void *ptr) const
{
    is_nullptr(ptr, "convertPtr()", "Null pointer.");
    T *converted = static_cast<T *>(const_cast<void *>(ptr));
    is_nullptr(converted, "convertPtr()", "fall to convert ptr!");
    return converted;
}

// 转化对象的数据指针类型
template <typename T>
T *DataBlob::convertPtr() const
{
    if (Blob_Data == nullptr || Blob_Data->data == nullptr)
    {
        throw std::invalid_argument("Null pointer");
    }
    T *converted = static_cast<T *>(const_cast<void *>(Blob_Data->data));
    is_nullptr(converted, "convertPtr()", "fall to convert ptr!");
    return converted;
}

template <typename T>
DataBlob DataBlob::add(const DataBlob &other) const
{
    T *data_this = convertPtr<T>();
    T *data_other = other.convertPtr<T>();
    T *result_data = new T[Blob_Data->length];
    for (size_t i = 0; i < Blob_Data->length; i++)
    {
        result_data[i] = data_this[i] + data_other[i];
    }
    return DataBlob(Blob_Data->rows, Blob_Data->cols, Blob_Data->channels, Blob_Data->type, result_data);
}

template <typename T>
DataBlob DataBlob::minus(const DataBlob &other) const
{
    T *data_this = convertPtr<T>();
    T *data_other = other.convertPtr<T>();
    T *result_data = new T[Blob_Data->length];
    for (size_t i = 0; i < Blob_Data->length; i++)
    {
        result_data[i] = data_this[i] - data_other[i];
    }
    return DataBlob(Blob_Data->rows, Blob_Data->cols, Blob_Data->channels, Blob_Data->type, result_data);
}

template <typename T>
DataBlob DataBlob::mul(const DataBlob &other) const
{
    T *data_this = convertPtr<T>();
    T *data_other = other.convertPtr<T>();
    size_t r_left = Blob_Data->rows;
    size_t c_left = Blob_Data->cols;
    size_t c_right = other.Blob_Data->cols;
    size_t num_ch = Blob_Data->channels;
    size_t loc_result = 0;
    T *result_data = new T[r_left * c_right];
    for (size_t ch = 0; ch < num_ch; ch++)
    {
        for (size_t r = 0; r < r_left; r++)
        {
            for (size_t c = 0; c < c_right; c++)
            {
                T result_rc = 0;
                size_t loc_left = r * c_left;
                size_t loc_right = c;
                for (size_t i = 0; i < c_left; i++)
                {
                    result_rc = result_rc + data_this[loc_left] * data_other[loc_right];
                    loc_left = loc_left + 1;
                    loc_right = loc_right + c_right;
                }
                result_data[loc_result] = result_rc;
                loc_result = loc_result + 1;
            }
        }
    }
    return DataBlob(Blob_Data->rows, other.Blob_Data->cols, Blob_Data->channels, Blob_Data->type, result_data);
}

DataBlob DataBlob::operator+(const DataBlob &other) const
{
    is_valid();
    other.is_valid();
    if (!data_dimension(other))
    {
        std::cerr << "DataBlob::operator+(): The size of two DataBlobs doesn't match." << std::endl;
        exit(EXIT_FAILURE);
    }
    switch (Blob_Data->type)
    {
    case DataType::INT:
    {
        return add<int>(other);
        break;
    }
    case DataType::DOUBLE:
    {
        return add<double>(other);
        break;
    }
    case DataType::FLOAT:
    {
        return add<float>(other);
        break;
    }
    case DataType::SHORT:
    {
        return add<short>(other);
        break;
    }
    case DataType::LONG:
    {
        return add<long>(other);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        return add<unsigned char>(other);
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type!");
    }
}

DataBlob DataBlob::operator-(const DataBlob &other) const
{
    is_valid();
    other.is_valid();
    if (!data_dimension(other))
    {
        std::cerr << "The size of two DataBlobs doesn't match." << std::endl;
        exit(EXIT_FAILURE);
    }
    switch (Blob_Data->type)
    {
    case DataType::INT:
    {
        return minus<int>(other);
        break;
    }
    case DataType::DOUBLE:
    {
        return minus<double>(other);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        return minus<unsigned char>(other);
        break;
    }
    case DataType::FLOAT:
    {
        return minus<float>(other);
        break;
    }
    case DataType::LONG:
    {
        return minus<long>(other);
        break;
    }
    case DataType::SHORT:
    {
        return minus<short>(other);
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type!");
    }
}

DataBlob DataBlob::operator*(const DataBlob &other) const
{
    is_valid();
    other.is_valid();
    if (!mul_dimension(other))
    {
        std::cerr << "The size of two DataBlobs doesn't match." << std::endl;
        exit(EXIT_FAILURE);
    }
    switch (Blob_Data->type)
    {
    case DataType::INT:
    {
        return mul<int>(other);
        break;
    }
    case DataType::DOUBLE:
    {
        return mul<double>(other);
        break;
    }
    case DataType::FLOAT:
    {
        return mul<float>(other);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        return mul<unsigned char>(other);
        break;
    }
    case DataType::LONG:
    {
        return mul<long>(other);
        break;
    }
    case DataType::SHORT:
    {
        return mul<short>(other);
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type for matrix multiplication!");
    }
}

template <typename T>
T DataBlob::at(size_t r, size_t c, size_t ch) const
{

    if (r < 0 || r >= Blob_Data->rows || c < 0 || c >= Blob_Data->cols || ch < 0 || ch >= Blob_Data->channels)
    {
        std::cerr << "DataBlob::at(): Index out of range!" << std::endl;
        exit(EXIT_FAILURE);
    }
    return *((T *)((char *)Blob_Data->data + (ch * Blob_Data->rows * Blob_Data->cols + (r * Blob_Data->cols + c)) * Blob_Data->type_size));
}

template <typename T>
void DataBlob::set(size_t r, size_t c, size_t ch, T value)
{
    is_nullptr(Blob_Data, "set()", "Blob_Data is nullptr.");
    if (r < 0 || r >= Blob_Data->rows || c < 0 || c >= Blob_Data->cols || ch < 0 || ch >= Blob_Data->channels)
    {
        std::cerr << "DataBlob::set(): Index out of range!" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (Blob_Data->referenceCounter->getCount() > 1)
    {
        Data *newData = Blob_Data->clone();
        Blob_Data->referenceCounter->decrease();
        Blob_Data = newData;
    }
    *((T *)Blob_Data->data + (r * Blob_Data->cols + c) + ch * Blob_Data->cols * Blob_Data->rows) = value;
}

template <typename T>
void DataBlob::set_all(size_t r, size_t c, size_t ch, T value)
{
    is_nullptr(Blob_Data, "set()", "Blob_Data is nullptr.");
    if (r < 0 || r >= Blob_Data->rows || c < 0 || c >= Blob_Data->cols || ch < 0 || ch >= Blob_Data->channels)
    {
        std::cerr << "DataBlob::set(): Index out of range!" << std::endl;
        exit(EXIT_FAILURE);
    }
    *((T *)Blob_Data->data + (r * Blob_Data->cols + c) + ch * Blob_Data->cols * Blob_Data->rows) = value;
}

void DataBlob::release()
{
    if (Blob_Data->referenceCounter->decrease())
    {
        delete Blob_Data;
        Blob_Data = nullptr; // 在多线程中更加安全
        // std::cout << "Data is delete!" << std::endl;
    }
}

// 对对象进行克隆
DataBlob DataBlob::clone() const
{
    DataBlob newBlob(Blob_Data->rows, Blob_Data->cols, Blob_Data->channels, Blob_Data->type);
    memcpy(newBlob.Blob_Data->data, Blob_Data->data, Blob_Data->rows * Blob_Data->cols * Blob_Data->channels * Blob_Data->type_size);
    return newBlob;
}

size_t DataBlob::get_reference_count()
{
    return Blob_Data->referenceCounter->getCount();
}

bool DataBlob::is_part(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end) const
{
    if (row_start < 0 || row_start >= Blob_Data->rows || col_start < 0 || col_start >= Blob_Data->cols || channels_start < 0 || channels_start >= Blob_Data->channels)
    {
        return false;
    }
    else
    {
        return true;
    }
}

DataBlob DataBlob::get_part(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end) const
{
    is_valid();
    if (!is_part(row_start, row_end, col_start, col_end, channels_start, channels_end))
    {
        std::cerr << "DataBlob::get_part(): Index out of range!" << std::endl;
        exit(EXIT_FAILURE);
    }
    switch (Blob_Data->type)
    {
    case DataType::INT:
    {
        return get_PART<int>(row_start, row_end, col_start, col_end, channels_start, channels_end);
        break;
    }
    case DataType::DOUBLE:
    {
        return get_PART<double>(row_start, row_end, col_start, col_end, channels_start, channels_end);
        break;
    }
    case DataType::FLOAT:
    {
        return get_PART<float>(row_start, row_end, col_start, col_end, channels_start, channels_end);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        return get_PART<unsigned char>(row_start, row_end, col_start, col_end, channels_start, channels_end);
        break;
    }
    case DataType::LONG:
    {
        return get_PART<long>(row_start, row_end, col_start, col_end, channels_start, channels_end);
        break;
    }
    case DataType::SHORT:
    {
        return get_PART<short>(row_start, row_end, col_start, col_end, channels_start, channels_end);
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type!");
    }
}

template <typename T>
DataBlob DataBlob::get_PART(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end) const
{
    is_valid();
    T *data_this = convertPtr<T>();
    size_t loc = channels_start * Blob_Data->rows * Blob_Data->cols + row_start * Blob_Data->cols + col_start;
    size_t loc_result = 0;
    size_t num_row, num_col, num_channels;
    int dif_row, dif_col, dif_channel;
    if (row_start < row_end)
    {
        dif_row = 1;
        num_row = row_end - row_start + 1;
    }
    else
    {
        dif_row = -1;
        num_row = row_start - row_end + 1;
    }
    if (col_start < col_end)
    {
        dif_col = 1;
        num_col = col_end - col_start + 1;
    }
    else
    {
        dif_col = -1;
        num_col = col_start - col_end + 1;
    }
    if (channels_start < channels_end)
    {
        dif_channel = 1;
        num_channels = channels_end - channels_start + 1;
    }
    else
    {
        dif_channel = -1;
        num_channels = channels_start - channels_end + 1;
    }
    T *result_data = new T[num_row * num_col * num_channels];

    for (size_t ch = 0; ch < num_channels; ch++)
    {
        for (size_t r = 0; r < num_row; r++)
        {
            for (size_t c = 0; c < num_col; c++)
            {
                result_data[loc_result] = data_this[loc];
                loc_result++;
                loc = loc + dif_col;
            }
            loc = loc + dif_row * Blob_Data->cols - num_col;
        }
        loc = loc + dif_channel * Blob_Data->rows * Blob_Data->cols - num_col * num_row;
    }
    return DataBlob(num_row, num_col, num_channels, Blob_Data->type, result_data);
}

void DataBlob::set_part_all(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end, const DataBlob &other) const
{
    is_valid();
    if (!is_part(row_start, row_end, col_start, col_end, channels_start, channels_end))
    {
        std::cerr << "DataBlob::set_part_all(): Index out of range!" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (this->Blob_Data->type != other.Blob_Data->type)
    {
        std::cerr << "DataBlob::set_part_all(): type not same!" << std::endl;
        exit(EXIT_FAILURE);
    }

    switch (Blob_Data->type)
    {
    case DataType::INT:
    {
        set_PART<int>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::DOUBLE:
    {
        set_PART<double>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::FLOAT:
    {
        set_PART<float>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        set_PART<unsigned char>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::LONG:
    {
        set_PART<long>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::SHORT:
    {
        set_PART<short>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type!");
    }
}

void DataBlob::set_part(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end, const DataBlob &other)
{
    is_valid();
    if (!is_part(row_start, row_end, col_start, col_end, channels_start, channels_end))
    {
        std::cerr << "DataBlob::get_part(): Index out of range!" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (this->Blob_Data->type != other.Blob_Data->type)
    {
        std::cerr << "DataBlob::set_part(): type not same!" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (Blob_Data->referenceCounter->getCount() > 1)
    {
        Data *newData = Blob_Data->clone();
        Blob_Data->referenceCounter->decrease();
        Blob_Data = newData;
    }
    switch (Blob_Data->type)
    {
    case DataType::INT:
    {
        set_PART<int>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::SHORT:
    {
        set_PART<short>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::LONG:
    {
        set_PART<long>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::DOUBLE:
    {
        set_PART<double>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::FLOAT:
    {
        set_PART<float>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        set_PART<unsigned char>(row_start, row_end, col_start, col_end, channels_start, channels_end, other);
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type!");
    }
}

template <typename T>
void DataBlob::set_PART(size_t row_start, size_t row_end, size_t col_start, size_t col_end, size_t channels_start, size_t channels_end, const DataBlob &other) const
{
    is_valid();
    other.is_valid();
    size_t num_row, num_col, num_channels;
    int dif_row, dif_col, dif_channel;
    if (row_start < row_end)
    {
        dif_row = 1;
        num_row = row_end - row_start + 1;
    }
    else
    {
        dif_row = -1;
        num_row = row_start - row_end + 1;
    }
    if (col_start < col_end)
    {
        dif_col = 1;
        num_col = col_end - col_start + 1;
    }
    else
    {
        dif_col = -1;
        num_col = col_start - col_end + 1;
    }
    if (channels_start < channels_end)
    {
        dif_channel = 1;
        num_channels = channels_end - channels_start + 1;
    }
    else
    {
        dif_channel = -1;
        num_channels = channels_start - channels_end + 1;
    }
    if (num_row != other.Blob_Data->rows || num_col != other.Blob_Data->cols || num_channels != other.Blob_Data->channels)
    {
        std::cerr << "DataBlob::set_part: Size inconsistency." << std::endl;
        exit(EXIT_FAILURE);
    }

    T *data_this = convertPtr<T>();
    T *data_other = other.convertPtr<T>();
    size_t loc_this = channels_start * Blob_Data->rows * Blob_Data->cols + row_start * Blob_Data->cols + col_start;
    size_t loc_other = 0;

    for (size_t ch = 0; ch < num_channels; ch++)
    {
        for (size_t r = 0; r < num_row; r++)
        {
            for (size_t c = 0; c < num_col; c++)
            {
                data_this[loc_this] = data_other[loc_other];
                loc_other++;
                loc_this = loc_this + dif_col;
            }
            loc_this = loc_this + dif_row * Blob_Data->cols - num_col;
        }
        loc_this = loc_this + dif_channel * Blob_Data->rows * Blob_Data->cols - num_col * num_row;
    }
}

template <typename T>
DataBlob DataBlob::transposition() const
{
    is_valid();
    T *data = convertPtr<T>();
    T *result_data = new T[Blob_Data->length];
    size_t result_col = Blob_Data->rows;
    size_t result_row = Blob_Data->cols;
    size_t result_channel = Blob_Data->channels;
    size_t loc_result = 0;

    for (size_t ch = 0; ch < result_channel; ch++)
    {
        for (size_t r = 0; r < result_row; r++)
        {
            for (size_t c = 0; c < result_col; c++)
            {
                result_data[loc_result] = data[ch * result_col * result_row + c * result_row + r];
                loc_result++;
            }
        }
    }
    return DataBlob(result_row, result_col, result_channel, Blob_Data->type, result_data);
}

DataBlob DataBlob::operator~() const
{
    switch (Blob_Data->type)
    {
    case DataType::INT:
    {
        return transposition<int>();
        break;
    }
    case DataType::DOUBLE:
    {
        return transposition<double>();
        break;
    }
    case DataType::FLOAT:
    {
        return transposition<float>();
        break;
    }
    case DataType::LONG:
    {
        return transposition<long>();
        break;
    }
    case DataType::SHORT:
    {
        return transposition<short>();
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        return transposition<unsigned char>();
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type for matrix multiplication!");
    }
}

template <typename T>
std::ostream &printData(std::ostream &os, const DataBlob blob)
{
    blob.is_valid();
    size_t num_channel = blob.Blob_Data->channels;
    size_t num_row = blob.Blob_Data->rows;
    size_t num_col = blob.Blob_Data->cols;
    T *data = blob.convertPtr<T>();
    size_t loc = 0;
    for (size_t ch = 0; ch < num_channel; ch++)
    {
        os << std::endl
           << "channel: " << ch << std::endl;
        for (size_t r = 0; r < num_row; r++)
        {
            for (size_t c = 0; c < num_col; c++)
            {
                os << data[loc] << ' ';
                loc++;
            }
            os << std::endl;
        }
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const DataBlob blob)
{
    if (blob.Blob_Data == nullptr)
    {
        std::cerr << "DataBlob:: <<: nullptr!" << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (blob.Blob_Data->data == nullptr)
    {
        std::cerr << "DataBlob:: <<: nullptr!" << std::endl;
        exit(EXIT_FAILURE);
    }

    switch (blob.Blob_Data->type)
    {
    case DataType::INT:
    {
        return printData<int>(os, blob);
        break;
    }
    case DataType::DOUBLE:
    {
        return printData<double>(os, blob);
        break;
    }
    case DataType::FLOAT:
    {
        return printData<float>(os, blob);
        break;
    }
    case DataType::LONG:
    {
        return printData<long>(os, blob);
        break;
    }
    case DataType::SHORT:
    {
        return printData<short>(os, blob);
        break;
    }
    case DataType::UNSIGNED_CHAR:
    {
        return printData<unsigned char>(os, blob);
        break;
    }
    default:
        throw std::invalid_argument("Invalid data type for matrix multiplication!");
    }
}

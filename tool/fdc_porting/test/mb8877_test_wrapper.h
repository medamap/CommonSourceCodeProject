#ifndef MB8877_TEST_WRAPPER_H
#define MB8877_TEST_WRAPPER_H

// Forward declarations
class MockEnvironment;
class MB8877;

// Test wrapper class for MB8877
class MB8877TestWrapper {
private:
    MockEnvironment* env;
    MB8877* fdc;
    
public:
    MB8877TestWrapper(MockEnvironment* environment);
    ~MB8877TestWrapper();
    
    MB8877* getFDC() { return fdc; }
    
    // Helper methods for disk operations
    void insertDisk(int drive, const char* filename);
    void ejectDisk(int drive);
    bool isDiskInserted(int drive);
};

#endif // MB8877_TEST_WRAPPER_H
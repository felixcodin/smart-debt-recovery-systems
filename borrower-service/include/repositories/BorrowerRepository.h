// BorrowerRepository.h - Data access layer for Borrower entities

#ifndef SDRS_BORROWER_REPOSITORY_H
#define SDRS_BORROWER_REPOSITORY_H

#include "../models/Borrower.h"
#include "../../../common/include/database/DatabaseManager.h"
#include <optional>
#include <vector>
#include <memory>

namespace sdrs::borrower
{

class BorrowerRepository
{
private:
    bool _useMock;

public:
    // Constructor: useMock=true for testing without database
    explicit BorrowerRepository(bool useMock = false);
    ~BorrowerRepository() = default;

    // CRUD operations
    Borrower create(const Borrower& borrower);
    std::optional<Borrower> findById(int id);
    Borrower update(const Borrower& borrower);
    bool deleteById(int id);
    std::vector<Borrower> findAll();
    
    // Additional queries
    std::optional<Borrower> findByEmail(const std::string& email);
    std::vector<Borrower> findByActiveStatus(bool isActive);
    int count();

private:
    // Mock implementations for testing
    Borrower createMock(const Borrower& borrower);
    std::optional<Borrower> findByIdMock(int id);
    Borrower updateMock(const Borrower& borrower);
    bool deleteByIdMock(int id);
    std::vector<Borrower> findAllMock();
    
    // Helper to map database row to Borrower object
    static Borrower mapRowToBorrower(const pqxx::row& row);
};

} // namespace sdrs::borrower

#endif // SDRS_BORROWER_REPOSITORY_H

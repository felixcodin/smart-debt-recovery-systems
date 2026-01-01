// LoanAccountRepository.h - Data access layer for LoanAccount entities

#ifndef SDRS_LOAN_ACCOUNT_REPOSITORY_H
#define SDRS_LOAN_ACCOUNT_REPOSITORY_H

#include "../models/LoanAccount.h"
#include "../../../common/include/database/DatabaseManager.h"
#include <optional>
#include <vector>

namespace sdrs::borrower
{

class LoanAccountRepository
{
private:
    bool _useMock;

public:
    explicit LoanAccountRepository(bool useMock = false);
    ~LoanAccountRepository() = default;

    // CRUD operations
    LoanAccount create(const LoanAccount& account);
    std::optional<LoanAccount> findById(int accountId);
    LoanAccount update(const LoanAccount& account);
    bool deleteById(int accountId);
    
    // Query operations
    std::vector<LoanAccount> findByBorrowerId(int borrowerId);
    std::vector<LoanAccount> findByStatus(sdrs::constants::AccountStatus status);
    std::vector<LoanAccount> findDelinquent(int minDaysPastDue = 1);
    std::vector<LoanAccount> findAll();
    int count();
    
    // Update specific fields
    bool updateStatus(int accountId, sdrs::constants::AccountStatus status);
    bool updateDaysPastDue(int accountId, int daysPastDue);
    bool recordPayment(int accountId, double amount);

private:
    // Mock implementations
    LoanAccount createMock(const LoanAccount& account);
    std::optional<LoanAccount> findByIdMock(int accountId);
    std::vector<LoanAccount> findByBorrowerIdMock(int borrowerId);
    std::vector<LoanAccount> findAllMock();
    
    // Helper
    static LoanAccount mapRowToLoanAccount(const pqxx::row& row);
};

} // namespace sdrs::borrower

#endif // SDRS_LOAN_ACCOUNT_REPOSITORY_H

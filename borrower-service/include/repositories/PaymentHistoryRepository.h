// PaymentHistoryRepository.h - PostgreSQL repository for PaymentHistory

#ifndef SDRS_PAYMENT_HISTORY_REPOSITORY_H
#define SDRS_PAYMENT_HISTORY_REPOSITORY_H

#include <vector>
#include <optional>
#include <string>
#include "../models/PaymentHistory.h"
#include "../../../common/include/database/DatabaseManager.h"

namespace sdrs::borrower
{

/**
 * @brief Repository for managing PaymentHistory entities in PostgreSQL
 * 
 * Provides CRUD operations and specialized queries for the payment_history table.
 * Supports mock mode for unit testing without database.
 */
class PaymentHistoryRepository
{
private:
    bool _useMock;
    
public:
    /**
     * @brief Construct repository with optional mock mode
     * @param useMock If true, use mock data instead of database
     */
    explicit PaymentHistoryRepository(bool useMock = false);
    
    // ========================================================================
    // CRUD Operations
    // ========================================================================
    
    /**
     * @brief Create a new payment record in database
     * @param payment PaymentHistory object to insert
     * @return Created PaymentHistory with generated ID
     */
    PaymentHistory create(const PaymentHistory& payment);
    
    /**
     * @brief Find payment by ID
     * @param paymentId Payment ID to search
     * @return PaymentHistory if found, nullopt otherwise
     */
    std::optional<PaymentHistory> findById(int paymentId);
    
    /**
     * @brief Update existing payment record
     * @param payment PaymentHistory with updated values
     * @return Updated PaymentHistory
     */
    PaymentHistory update(const PaymentHistory& payment);
    
    /**
     * @brief Delete payment record by ID
     * @param paymentId ID to delete
     * @return true if deleted, false if not found
     */
    bool deleteById(int paymentId);
    
    // ========================================================================
    // Query Operations
    // ========================================================================
    
    /**
     * @brief Find all payments for a specific loan account
     * @param accountId Loan account ID
     * @return Vector of payments ordered by payment_date DESC
     */
    std::vector<PaymentHistory> findByAccountId(int accountId);
    
    /**
     * @brief Find payments by status
     * @param status Payment status to filter
     * @return Vector of payments with matching status
     */
    std::vector<PaymentHistory> findByStatus(sdrs::constants::PaymentStatus status);
    
    /**
     * @brief Find late payments
     * @return Vector of payments where is_late = true
     */
    std::vector<PaymentHistory> findLatePayments();
    
    /**
     * @brief Find payments within a date range
     * @param startDate Start date (YYYY-MM-DD format)
     * @param endDate End date (YYYY-MM-DD format)
     * @return Vector of payments within range
     */
    std::vector<PaymentHistory> findByDateRange(const std::string& startDate, const std::string& endDate);
    
    /**
     * @brief Find pending payments (not yet processed)
     * @return Vector of pending payments
     */
    std::vector<PaymentHistory> findPendingPayments();
    
    /**
     * @brief Get all payments (limited to 100)
     * @return Vector of all payments
     */
    std::vector<PaymentHistory> findAll();
    
    /**
     * @brief Count total payments in database
     * @return Total count
     */
    int count();
    
    /**
     * @brief Sum total payments for an account
     * @param accountId Loan account ID
     * @return Total amount paid
     */
    double sumPaymentsForAccount(int accountId);
    
    // ========================================================================
    // Update Operations
    // ========================================================================
    
    /**
     * @brief Update payment status
     * @param paymentId Payment ID
     * @param status New status
     * @return true if updated
     */
    bool updateStatus(int paymentId, sdrs::constants::PaymentStatus status);
    
    /**
     * @brief Mark payment as late
     * @param paymentId Payment ID
     * @param isLate Late flag
     * @return true if updated
     */
    bool markAsLate(int paymentId, bool isLate = true);
    
    // ========================================================================
    // Helper Methods
    // ========================================================================
    
    /**
     * @brief Map database row to PaymentHistory object
     * @param row pqxx result row
     * @return PaymentHistory object
     */
    static PaymentHistory mapRowToPaymentHistory(const pqxx::row& row);
    
    // ========================================================================
    // Mock Methods (for testing)
    // ========================================================================
    
private:
    PaymentHistory createMock(const PaymentHistory& payment);
    std::optional<PaymentHistory> findByIdMock(int paymentId);
    std::vector<PaymentHistory> findByAccountIdMock(int accountId);
    std::vector<PaymentHistory> findAllMock();
};

} // namespace sdrs::borrower

#endif // SDRS_PAYMENT_HISTORY_REPOSITORY_H

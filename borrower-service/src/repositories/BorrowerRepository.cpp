// BorrowerRepository.cpp - Mock implementation for MVP

#include "../../include/repositories/BorrowerRepository.h"
#include "../../../common/include/utils/Logger.h"

namespace sdrs::borrower
{

Borrower BorrowerRepository::create(const Borrower& borrower)
{
    // MVP: Return borrower with mock ID
    sdrs::utils::Logger::Info("[MOCK] Creating borrower: " + borrower.getFirstName() + " " + borrower.getLastName());
    
    // In real implementation, would insert to database and return with generated ID
    Borrower created(999, borrower.getFirstName(), borrower.getLastName());
    created.setEmail(borrower.getEmail());
    created.setPhoneNumber(borrower.getPhoneNumber());
    created.setAddress(borrower.getAddress());
    created.setActive();
    
    return created;
}

std::optional<Borrower> BorrowerRepository::findById(int id)
{
    sdrs::utils::Logger::Info("[MOCK] Finding borrower by ID: " + std::to_string(id));
    
    // MVP: Return mock borrower if ID is valid
    if (id <= 0)
    {
        return std::nullopt;
    }
    
    // Create a mock borrower
    Borrower borrower(id, "John", "Doe");
    borrower.setEmail("john.doe@example.com");
    borrower.setPhoneNumber("0123456789");
    borrower.setAddress("123 Main St, Hanoi");
    borrower.setMonthlyIncome(15000000.0);
    borrower.setActive();
    
    return borrower;
}

Borrower BorrowerRepository::update(const Borrower& borrower)
{
    sdrs::utils::Logger::Info("[MOCK] Updating borrower ID: " + std::to_string(borrower.getId()));
    
    // MVP: Return the same borrower (simulating successful update)
    return borrower;
}

bool BorrowerRepository::deleteById(int id)
{
    sdrs::utils::Logger::Info("[MOCK] Deleting borrower ID: " + std::to_string(id));
    
    // MVP: Always return true (simulating successful deletion)
    return id > 0;
}

std::vector<Borrower> BorrowerRepository::findAll()
{
    sdrs::utils::Logger::Info("[MOCK] Finding all borrowers");
    
    // MVP: Return a list with 2 mock borrowers
    std::vector<Borrower> borrowers;
    
    Borrower b1(1, "Alice", "Smith");
    b1.setEmail("alice.smith@example.com");
    b1.setActive();
    borrowers.push_back(b1);
    
    Borrower b2(2, "Bob", "Johnson");
    b2.setEmail("bob.johnson@example.com");
    b2.setActive();
    borrowers.push_back(b2);
    
    return borrowers;
}

} // namespace sdrs::borrower

// BorrowerRepository.h - Data access layer for Borrower entities

#ifndef SDRS_BORROWER_REPOSITORY_H
#define SDRS_BORROWER_REPOSITORY_H

#include "../models/Borrower.h"
#include <optional>
#include <vector>
#include <memory>

namespace sdrs::borrower
{

class BorrowerRepository
{
public:
    BorrowerRepository() = default;
    ~BorrowerRepository() = default;

    // CRUD operations (MVP: mock implementations)
    Borrower create(const Borrower& borrower);
    std::optional<Borrower> findById(int id);
    Borrower update(const Borrower& borrower);
    bool deleteById(int id);
    std::vector<Borrower> findAll();
};

} // namespace sdrs::borrower

#endif // SDRS_BORROWER_REPOSITORY_H

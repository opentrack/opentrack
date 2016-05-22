#pragma once

class check_for_first_run final
{
    bool checked_for_first_run;
    bool is_first_instance;
    bool enabled;
    
    void try_exit();
public:
    check_for_first_run();
    bool is_first_run();
    void set_enabled(bool flag);
    void try_runonce();
    ~check_for_first_run();
};


#define impl_cast(x) ((array_t*)(x))
//helphelphelphelp
static array_status attempt_repair(array_t *self)    {
    switch(check_valid(self))   {
        case SUCCESS: return SUCCESS;
        case STATE_INVAL:
        break;
        case NULL_ARG:
            DBG_LOG("self was null\n");
            return NULL_ARG;
        break;
        case NULL_IMPL:
            DBG_LOG("lost ref to impl cleanly (null)... memory leak likely.");
            impl                = malloc(sizeof(array_t));
            if(impl == NULL)    {
                DBG_LOG("Could not malloc new impl");
                return STATE_INVAL;
            }

        case NULL_BUF:  // FALLTHROUGH INTENTIONAL
            impl                = impl_cast(self->impl);
            impl->data          = NULL; // insurance
            impl->data          = create_dynabuf(REPAIR_SIZE);
            if(impl->data == NULL)  {
                DBG_LOG("Could not create dynabuf with new size %d\n",
                        REPAIR_SIZE);
                return STATE_INVAL;
            }
            impl->size          = 0;
            self->impl          = impl;
            return SUCCESS;
        break;

        default:
            return STATE_INVAL;
    }
    return STATE_INVAL;
}

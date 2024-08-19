#include "../mini_shell.h"

static void cleanup_pipes(int pipe_count, int pipe_fds[][2])
{
    for (int i = 0; i < pipe_count; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }
}
//=-=-=-=-=

static int setup_execution(setup_execution_params *params) {
    *(params->command_count) = count_commands(params->cmd);
    *(params->pipe_count) = *(params->command_count) - 1;

    save_original_io(params->io);

    if (setup_pipes(*(params->pipe_count), params->pipe_fds) != 0) {
        restore_io(params->io);
        return 1;
    }
    return 0;
}

static int setup_and_check_heredoc(heredoc_setup_params *params) {
    (void)(params->command_count);
    *(params->heredoc_count) = count_heredocs(params->cmd->red);
    *(params->heredoc_fds) = NULL;
    if (*(params->heredoc_count) > 0)
    {
        *(params->heredoc_fds) = handle_heredocs(params->cmd->red, *(params->heredoc_count), params->env);
        if (!*(params->heredoc_fds)) {
            restore_io(params->io);
            return 1;
        }
    }
    return 0;
}

int execute_commands_loop(execute_loop_params *params) {
    int cmd_index = 0;
    while (params->cmd) {
        int heredoc_count;
        int *heredoc_fds;

        // In execute_commands_loop function:
        heredoc_setup_params heredoc_params = {
        params->cmd, params->env, params->io, &heredoc_count, &heredoc_fds, params->command_count
        };
        int setup_result = setup_and_check_heredoc(&heredoc_params);
        // int setup_result = setup_and_check_heredoc(params->cmd, params->env, params->io, &heredoc_count, &heredoc_fds, params->command_count);
        if (setup_result != 0)
            return setup_result;

        fork_execute_params fork_params = {
            params->cmd, params->env, params->exit_status, cmd_index,
            params->pipe_count, params->pipe_fds, heredoc_fds, &(params->pids[cmd_index])
        };
        if (fork_and_execute(&fork_params) != 0) {
            restore_io(params->io);
            return 1;
        }

        cleanup_parent_process(heredoc_fds, heredoc_count);
        params->cmd = params->cmd->next;
        cmd_index++;
    }
    return 0;
}
//=-=-=--=

//=-=-=-=
int execute_command(t_arg *cmd, t_env *env, int *exit_status) {
    t_io io;
    int command_count, pipe_count;
    int pipe_fds[MAX_PIPES][2];
    pid_t pids[MAX_COMMANDS];

    setup_execution_params setup_params = {
        cmd, &command_count, &pipe_count, pipe_fds, &io
    };
    if (setup_execution(&setup_params) != 0) {
        return 1;
    }

    int cmd_index = 0;
    while (cmd)
    {
        int heredoc_count = count_heredocs(cmd->red);
        int *heredoc_fds = NULL;
        if (heredoc_count > 0)
        {
            heredoc_fds = handle_heredocs(cmd->red, heredoc_count, env);
            if (!heredoc_fds)
            {
                restore_io(&io);
                return 1;
            }
        }
        else if (command_count == 1 && is_builtin(cmd->arg[0])) {
            if (apply_redirections(cmd->red) == -1) {
                restore_io(&io);
                return 1;
            }
            int result = execute_builtin(cmd, env, exit_status);
            restore_io(&io);
            return result;
        }

        fork_execute_params fork_params = {
            cmd, env, exit_status, cmd_index, pipe_count, pipe_fds, heredoc_fds, &pids[cmd_index]
        };
        if (fork_and_execute(&fork_params) != 0) {
            restore_io(&io);
            return 1;
        }

       g_sig.pid = pids[cmd_index];////////

        cleanup_parent_process(heredoc_fds, heredoc_count);

        cmd = cmd->next;
        cmd_index++;
    }

    cleanup_pipes(pipe_count, pipe_fds);

    int result = wait_for_children(pids, command_count, exit_status);

    // g_sig.exit_status = result;//////exitsssttatus

    restore_io(&io);
    return result;
}

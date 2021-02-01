// Overplot functions
function [%gca] = overplot(st, c, %title, %xlabel, %zoom, %ylim)
    xtitle(%title);
    if ~exists('%zoom') then
        %zoom = [];
    end
    [n, m] = size(st);
    [nc, mc] = size(c);
    for i=1:m
        tim = evstr(st(i)+'.time');
        val = evstr(st(i)+'.values');
        if ~isempty(%zoom) then
            indx1 = find(tim>=%zoom(1));
            indx2 = find(tim<=%zoom(2));
            indx = intersect(find(tim>=%zoom(1)), find(tim<=%zoom(2)));
            if indx then
                tim = tim(indx);
                val = val(indx);
            end
        end
        plot(tim, val, c(i), 'LineWidth', 2);
    end
    %gca = gca();   // return handle
    if exists('%ylim') then   // overrides zoom y values
        current_bounds = %gca.data_bounds;
        current_x_bounds = current_bounds(:,1)';
        current_y_bounds = current_bounds(:,2)';
        if size(%ylim,2)==2 then
            %gca.data_bounds = [current_x_bounds %ylim];
        else
            %ylim
            printf('WARNING(overplot):  ylim needs to be 1x2\nIgnoring and continuing---\n', %ylim)
        end
    end
    set(gca(),"grid",[1 1]);
    legend(st);
    if exists('%xlabel') then
        xlabel(%xlabel);
    end
endfunction

function [%gca] = scatterplot(st, c, %title, %xlabel, %zoom, %ylim)
    xtitle(%title);
    if ~exists('%zoom') then
        %zoom = [];
    end
    [n, m] = size(st);
    [nc, mc] = size(c);
    tim = evstr(st(1)+'.values');
    for i=2:m
        val = evstr(st(i)+'.values');
        if ~isempty(%zoom) then
            indx1 = find(tim>=%zoom(1));
            indx2 = find(tim<=%zoom(2));
            indx = intersect(find(tim>=%zoom(1)), find(tim<=%zoom(2)));
            if indx then
                tim = tim(indx);
                val = val(indx);
            end
        end
        plot(tim, val, c(i-1));
        st_scat(i-1) = st(i)+'_v_'+st(1);
    end
    %gca = gca();   // return handle
    if exists('%ylim') then   // overrides zoom y values
        current_bounds = %gca.data_bounds;
        current_x_bounds = current_bounds(:,1)';
        current_y_bounds = current_bounds(:,2)';
        if size(%ylim,2)==2 then
            %gca.data_bounds = [current_x_bounds %ylim];
        else
            %ylim
            printf('WARNING(scatterplot):  ylim needs to be 1x2\nIgnoring and continuing---\n', %ylim)
        end
    end
    set(gca(),"grid",[1 1]);
    legend(st_scat, 3);
    if exists('%xlabel') then
        xlabel(%xlabel);
    end
endfunction

// Zoom all figure handles in time
function zoom_all(%figs, %zoom)
    for i = 1:length(%figs)
        show_window(%figs(i))
        n_c = size(%figs(i).children, 1);
        if exists('%zoom') then
            for j = 1:n_c
                current_bounds = %figs(i).children(j).data_bounds;
                current_x_bounds = current_bounds(:,1)';
                if size(%zoom,2)==2 then
                    if %zoom(1)>=current_x_bounds(1) && %zoom(2)<=current_x_bounds(2) then
                        %figs(i).children(j).data_bounds([1 2]) = %zoom;
                    end
                else
                    %zoom
                    printf('WARNING(overplot):  zoom needs to be 1x2\nIgnoring and continuing---\n', %zoom)
                end
            end
        end
    end
endfunction

// Autoscale all
function unzoom_all(%figs)
    for i = 1:length(%figs)
        show_window(%figs(i))
        n_c = size(%figs(i).children, 1);
        for j = 1:n_c
            replot(%figs(i).children(j));
        end
    end
endfunction
